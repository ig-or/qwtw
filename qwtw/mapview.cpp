#include "mapview.h"
#include "line.h"
#include "qwproc.h"
#include "settings.h"

#include <qwt_plot_grid.h>
#include <qwt_plot_item.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_map.h>

#include <QCache>
#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFontMetrics>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmap>
#include <QSet>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>

namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr int kMinZoom = 0;
constexpr int kMaxZoom = 19;
constexpr double kTileSizePx = 256.0;

inline quint64 makeTileKey(int z, int x, int y) {
	return (static_cast<quint64>(z & 0xFF) << 48) |
	       (static_cast<quint64>(static_cast<uint32_t>(x) & 0xFFFFFFu) << 24) |
	       (static_cast<quint64>(static_cast<uint32_t>(y) & 0xFFFFFFu));
}

inline double tileXToLon(int x, int z) {
	const double n = static_cast<double>(1 << z);
	return (static_cast<double>(x) / n) * 360.0 - 180.0;
}

inline double tileYToLat(int y, int z) {
	const double n = static_cast<double>(1 << z);
	const double mercDeg = 180.0 - (static_cast<double>(y) / n) * 360.0;
	return MercatorLatTransform::mercatorDegToLat(mercDeg);
}

/**
 * Shared GUI-thread Slippy-Map tile downloader + 2-level cache (RAM + ~/.qwtw/tiles/).
 */
class OsmTileManager : public QObject {
public:
	static OsmTileManager& instance() {
		static OsmTileManager inst;
		return inst;
	}

	void subscribe(QwtPlot* plot) {
		if (plot) {
			subscribers_.insert(plot);
		}
	}

	void unsubscribe(QwtPlot* plot) {
		subscribers_.remove(plot);
	}

	bool hasPendingRequests() const {
		return !inflight_.isEmpty();
	}

	QString tileUrlTemplate() const {
		if (!qwSettings.map_tile_url.empty()) {
			return QString::fromStdString(qwSettings.map_tile_url);
		}
		return QStringLiteral("https://tile.openstreetmap.org/{z}/{x}/{y}.png");
	}

	const QPixmap* getTile(int z, int x, int y, bool requestIfMissing) {
		ensureProviderCurrent();
		const quint64 key = makeTileKey(z, x, y);

		if (const QPixmap* cached = memCache_.object(key)) {
			return cached;
		}

		const QString diskPath = tileDiskPath(z, x, y);
		if (!diskPath.isEmpty() && QFile::exists(diskPath)) {
			QPixmap pm;
			if (pm.load(diskPath) && !pm.isNull()) {
				auto* heapPm = new QPixmap(std::move(pm));
				memCache_.insert(key, heapPm, 1);
				return memCache_.object(key);
			}
		}

		if (requestIfMissing && !inflight_.contains(key) && !failed_.contains(key) && inflight_.size() < 48) {
			startDownload(key, z, x, y, diskPath);
		}
		return nullptr;
	}

	bool waitForPending(int timeoutMs) {
		if (inflight_.isEmpty()) {
			return true;
		}
		QEventLoop loop;
		QTimer timer;
		timer.setSingleShot(true);
		QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
		auto conn = QObject::connect(&coalesceTimer_, &QTimer::timeout, &loop, [&]() {
			if (inflight_.isEmpty()) {
				loop.quit();
			}
		});
		timer.start(timeoutMs);
		loop.exec();
		QObject::disconnect(conn);
		return inflight_.isEmpty();
	}

private:
	OsmTileManager() : memCache_(512) {
		coalesceTimer_.setSingleShot(true);
		coalesceTimer_.setInterval(16);
		QObject::connect(&coalesceTimer_, &QTimer::timeout, this, [this]() {
			for (QwtPlot* plt : subscribers_) {
				if (plt && plt->isVisible()) {
					plt->replot();
				}
			}
		});
		initBaseCacheDir();
		ensureProviderCurrent();
	}

	void initBaseCacheDir() {
		char folder[512] = {0};
		if (getFolderLocation(folder, sizeof(folder))) {
			baseTilesDir_ = QDir(QString::fromUtf8(folder)).filePath(QStringLiteral("tiles"));
		} else {
			baseTilesDir_ = QDir::temp().filePath(QStringLiteral("qwtw_tiles"));
		}
	}

	void ensureProviderCurrent() {
		const QString tpl = tileUrlTemplate();
		if (tpl == activeTemplate_) {
			return;
		}
		activeTemplate_ = tpl;
		const QByteArray hash = QCryptographicHash::hash(tpl.toUtf8(), QCryptographicHash::Sha1).toHex().left(12);
		providerCacheDir_ = QDir(baseTilesDir_).filePath(QString::fromLatin1(hash));
		memCache_.clear();
		failed_.clear();
	}

	QString tileDiskPath(int z, int x, int y) const {
		if (providerCacheDir_.isEmpty()) {
			return QString();
		}
		return QStringLiteral("%1/%2/%3/%4.png").arg(providerCacheDir_).arg(z).arg(x).arg(y);
	}

	void startDownload(quint64 key, int z, int x, int y, const QString& diskPath) {
		QString urlStr = activeTemplate_;
		urlStr.replace(QStringLiteral("{z}"), QString::number(z));
		urlStr.replace(QStringLiteral("{x}"), QString::number(x));
		urlStr.replace(QStringLiteral("{y}"), QString::number(y));

		QUrl url(urlStr);
		if (!url.isValid() || url.scheme().isEmpty()) {
			failed_.insert(key);
			return;
		}

		QNetworkRequest req(url);
		req.setHeader(QNetworkRequest::UserAgentHeader,
			QStringLiteral("qwtw-plot/3.0 (Qt6; https://github.com/ig-or/qwtw)"));
		req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
		req.setTransferTimeout(8000);

		inflight_.insert(key);
		QNetworkReply* reply = nam_.get(req);
		QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, key, diskPath]() {
			inflight_.remove(key);
			reply->deleteLater();

			if (reply->error() == QNetworkReply::NoError) {
				const QByteArray data = reply->readAll();
				QPixmap pm;
				if (pm.loadFromData(data) && !pm.isNull()) {
					if (!diskPath.isEmpty()) {
						QFileInfo fi(diskPath);
						QDir().mkpath(fi.absolutePath());
						QFile f(diskPath);
						if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
							f.write(data);
							f.close();
						}
					}
					memCache_.insert(key, new QPixmap(std::move(pm)), 1);
				} else {
					failed_.insert(key);
				}
			} else {
				xmprintf(6, "OsmTileManager: tile download failed (%s)\n",
					reply->errorString().toStdString().c_str());
				failed_.insert(key);
			}

			if (!coalesceTimer_.isActive()) {
				coalesceTimer_.start();
			}
		});
	}

	QNetworkAccessManager nam_;
	QCache<quint64, QPixmap> memCache_;
	QSet<quint64> inflight_;
	QSet<quint64> failed_;
	QSet<QwtPlot*> subscribers_;
	QTimer coalesceTimer_;
	QString baseTilesDir_;
	QString activeTemplate_;
	QString providerCacheDir_;
};

} // namespace

// ---------------------------------------------------------------------------
// MercatorLatTransform
// ---------------------------------------------------------------------------

double MercatorLatTransform::latToMercatorDeg(double latDeg) {
	const double clamped = std::clamp(latDeg, -kMaxLat, kMaxLat);
	const double rad = clamped * (kPi / 180.0);
	return (180.0 / kPi) * std::log(std::tan(kPi * 0.25 + rad * 0.5));
}

double MercatorLatTransform::mercatorDegToLat(double mercDeg) {
	const double clamped = std::clamp(mercDeg, -180.0, 180.0);
	return (360.0 / kPi) * std::atan(std::exp(clamped * (kPi / 180.0))) - 90.0;
}

double MercatorLatTransform::bounded(double value) const {
	return std::clamp(value, -kMaxLat, kMaxLat);
}

double MercatorLatTransform::transform(double value) const {
	return latToMercatorDeg(value);
}

double MercatorLatTransform::invTransform(double value) const {
	return mercatorDegToLat(value);
}

QwtTransform* MercatorLatTransform::copy() const {
	return new MercatorLatTransform();
}

// ---------------------------------------------------------------------------
// OsmTileItem
// ---------------------------------------------------------------------------

class OsmTileItem : public QwtPlotItem {
public:
	OsmTileItem() {
		setZ(1.0); // Below QwtPlotGrid (10), QwtPlotCurve (20), QwtPlotMarker (30)
		setItemAttribute(QwtPlotItem::AutoScale, false);
		setItemAttribute(QwtPlotItem::Legend, false);
		setRenderHint(QwtPlotItem::RenderAntialiased, true);
	}

	void draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const override {
		if (!painter || canvasRect.width() <= 2.0 || canvasRect.height() <= 2.0) {
			return;
		}

		double lonMin = std::min(xMap.s1(), xMap.s2());
		double lonMax = std::max(xMap.s1(), xMap.s2());
		double latMin = std::min(yMap.s1(), yMap.s2());
		double latMax = std::max(yMap.s1(), yMap.s2());

		lonMin = std::clamp(lonMin, -180.0, 180.0);
		lonMax = std::clamp(lonMax, -180.0, 180.0);
		latMin = std::clamp(latMin, -MercatorLatTransform::kMaxLat, MercatorLatTransform::kMaxLat);
		latMax = std::clamp(latMax, -MercatorLatTransform::kMaxLat, MercatorLatTransform::kMaxLat);

		const double lonSpan = std::max(1e-7, lonMax - lonMin);

		// Choose Slippy-Map zoom level z so 1 tile pixel ~= 1 logical canvas pixel
		const double idealTilesAcross = canvasRect.width() / kTileSizePx;
		const double rawZ = std::log2((idealTilesAcross * 360.0) / lonSpan);
		int z = std::clamp(static_cast<int>(std::floor(rawZ + 0.5)), kMinZoom, kMaxZoom);

		// Guard against excessive tile counts if aspect ratio is temporarily skewed
		while (z > kMinZoom) {
			const int n = 1 << z;
			const int tx0 = std::clamp(static_cast<int>(std::floor(((lonMin + 180.0) / 360.0) * n)), 0, n - 1);
			const int tx1 = std::clamp(static_cast<int>(std::floor(((lonMax + 180.0) / 360.0) * n)), 0, n - 1);
			const double mTop = MercatorLatTransform::latToMercatorDeg(latMax);
			const double mBot = MercatorLatTransform::latToMercatorDeg(latMin);
			const int ty0 = std::clamp(static_cast<int>(std::floor(((180.0 - mTop) / 360.0) * n)), 0, n - 1);
			const int ty1 = std::clamp(static_cast<int>(std::floor(((180.0 - mBot) / 360.0) * n)), 0, n - 1);
			if (static_cast<long long>(tx1 - tx0 + 1) * static_cast<long long>(ty1 - ty0 + 1) <= 80) {
				break;
			}
			--z;
		}

		const int n = 1 << z;
		const int xStart = std::clamp(static_cast<int>(std::floor(((lonMin + 180.0) / 360.0) * n)), 0, n - 1);
		const int xEnd   = std::clamp(static_cast<int>(std::floor(((lonMax + 180.0) / 360.0) * n)), 0, n - 1);

		const double mTop = MercatorLatTransform::latToMercatorDeg(latMax);
		const double mBot = MercatorLatTransform::latToMercatorDeg(latMin);
		const int yStart = std::clamp(static_cast<int>(std::floor(((180.0 - mTop) / 360.0) * n)), 0, n - 1);
		const int yEnd   = std::clamp(static_cast<int>(std::floor(((180.0 - mBot) / 360.0) * n)), 0, n - 1);

		painter->save();
		painter->setClipRect(canvasRect);
		painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
		painter->fillRect(canvasRect, QColor(238, 242, 235));

		OsmTileManager& mgr = OsmTileManager::instance();

		for (int ty = yStart; ty <= yEnd; ++ty) {
			const double latNorth = tileYToLat(ty, z);
			const double latSouth = tileYToLat(ty + 1, z);
			const double pyTop = yMap.transform(latNorth);
			const double pyBot = yMap.transform(latSouth);

			for (int tx = xStart; tx <= xEnd; ++tx) {
				const double lonWest = tileXToLon(tx, z);
				const double lonEast = tileXToLon(tx + 1, z);
				const double pxLeft  = xMap.transform(lonWest);
				const double pxRight = xMap.transform(lonEast);

				const QRectF dstRect(
					std::min(pxLeft, pxRight),
					std::min(pyTop, pyBot),
					std::fabs(pxRight - pxLeft),
					std::fabs(pyBot - pyTop));

				if (const QPixmap* pm = mgr.getTile(z, tx, ty, true)) {
					painter->drawPixmap(dstRect, *pm, QRectF(0.0, 0.0, pm->width(), pm->height()));
					continue;
				}

				// Progressive fallback: scale sub-region of closest cached ancestor tile while downloading
				bool drewFallback = false;
				for (int delta = 1; delta <= std::min(z, 5); ++delta) {
					const int pz = z - delta;
					const int px = tx >> delta;
					const int py = ty >> delta;
					if (const QPixmap* parentPm = mgr.getTile(pz, px, py, false)) {
						const int subDiv = 1 << delta;
						const int rx = tx - (px << delta);
						const int ry = ty - (py << delta);
						const double sw = static_cast<double>(parentPm->width()) / subDiv;
						const double sh = static_cast<double>(parentPm->height()) / subDiv;
						painter->drawPixmap(dstRect, *parentPm, QRectF(rx * sw, ry * sh, sw, sh));
						drewFallback = true;
						break;
					}
				}

				if (!drewFallback) {
					painter->setPen(QPen(QColor(215, 220, 212), 1.0));
					painter->drawRect(dstRect);
				}
			}
		}

		// Standard OpenStreetMap attribution badge in bottom-right corner
		const QString attr = QStringLiteral("\u00A9 OpenStreetMap contributors");
		QFont attrFont = painter->font();
		attrFont.setPointSize(8);
		painter->setFont(attrFont);
		const QFontMetrics fm(attrFont);
		const int tw = fm.horizontalAdvance(attr) + 10;
		const int th = fm.height() + 4;
		const QRectF badgeRect(
			canvasRect.right() - tw - 4.0,
			canvasRect.bottom() - th - 4.0,
			tw,
			th);
		painter->fillRect(badgeRect, QColor(255, 255, 255, 195));
		painter->setPen(QColor(50, 50, 50));
		painter->drawText(badgeRect, Qt::AlignCenter, attr);

		painter->restore();
	}
};

// ---------------------------------------------------------------------------
// MapView
// ---------------------------------------------------------------------------

MapView::MapView(const std::string& key_, XQPlots* pf_, QWidget* parent, unsigned int flags_)
	: Figure2(key_, pf_, parent, flags_ & ~3u, jMarble) {
	resize(640, 480);

	// Install Web Mercator transform on Y (latitude) axis
	auto* latEngine = new QwtLinearScaleEngine();
	latEngine->setTransformation(new MercatorLatTransform());
	plot1->setAxisScaleEngine(QwtPlot::yLeft, latEngine);

	// Soften grid lines so street names on tiles stay crisp and legible
	for (QwtPlotItem* item : plot1->itemList(QwtPlotItem::Rtti_PlotGrid)) {
		if (auto* grid = static_cast<QwtPlotGrid*>(item)) {
			grid->setMajorPen(QPen(QColor(90, 90, 90, 95), 0, Qt::DotLine));
			grid->setMinorPen(QPen(QColor(120, 120, 120, 55), 0, Qt::DotLine));
		}
	}

	tileItem_ = new OsmTileItem();
	tileItem_->attach(plot1);
	OsmTileManager::instance().subscribe(plot1);

	xlabel("Longitude [deg]");
	ylabel("Latitude [deg]");

	// Enable 1:1 conformal Web Mercator aspect ratio by default
	tbSquareAxis->setChecked(true);

	// Initial default map region before trajectories are plotted
	plot1->setAxisScale(QwtPlot::xBottom, -15.0, 45.0);
	plot1->setAxisScale(QwtPlot::yLeft, 34.0, 62.0);
	plot1->setAxisSquare(true);
	zoomer->setZoomBase(false);
	plot1->replot();
}

MapView::~MapView() {
	OsmTileManager::instance().unsubscribe(plot1);
	if (tileItem_) {
		tileItem_->detach();
		delete tileItem_;
		tileItem_ = nullptr;
	}
}

void MapView::fitToLines() {
	if (lines.empty()) {
		return;
	}

	double lonMin = 1e18;
	double lonMax = -1e18;
	double latMin = 1e18;
	double latMax = -1e18;
	bool found = false;

	auto accumulate = [&](bool requireImportant) {
		for (FigureItem* fi : lines) {
			LineItemInfo* info = fi->info;
			if (!info || info->size <= 0) {
				continue;
			}
			if (requireImportant && !info->important) {
				continue;
			}
			for (long long k = 0; k < info->size; ++k) {
				const double lon = info->x[k];
				const double lat = info->y[k];
				if (!std::isfinite(lon) || !std::isfinite(lat)) {
					continue;
				}
				lonMin = std::min(lonMin, lon);
				lonMax = std::max(lonMax, lon);
				latMin = std::min(latMin, lat);
				latMax = std::max(latMax, lat);
				found = true;
			}
		}
	};

	accumulate(true);
	if (!found) {
		accumulate(false);
	}
	if (!found) {
		return;
	}

	lonMin = std::clamp(lonMin, -180.0, 180.0);
	lonMax = std::clamp(lonMax, -180.0, 180.0);
	latMin = std::clamp(latMin, -MercatorLatTransform::kMaxLat, MercatorLatTransform::kMaxLat);
	latMax = std::clamp(latMax, -MercatorLatTransform::kMaxLat, MercatorLatTransform::kMaxLat);

	double mMin = MercatorLatTransform::latToMercatorDeg(latMin);
	double mMax = MercatorLatTransform::latToMercatorDeg(latMax);

	// Add 15% visual padding around trajectory (with ~450m minimum span for single points)
	constexpr double kMinSpanDeg = 0.004;
	const double lonPad = std::max((lonMax - lonMin) * 0.15, kMinSpanDeg * 0.5);
	const double mPad   = std::max((mMax - mMin) * 0.15, kMinSpanDeg * 0.5);

	lonMin = std::clamp(lonMin - lonPad, -180.0, 180.0);
	lonMax = std::clamp(lonMax + lonPad, -180.0, 180.0);
	mMin   = std::clamp(mMin - mPad, -180.0, 180.0);
	mMax   = std::clamp(mMax + mPad, -180.0, 180.0);

	// Pre-fit to canvas aspect ratio in Mercator coordinates if conformal mode is active
	if (tbSquareAxis->isChecked() && plot1->canvas() && plot1->canvas()->width() > 10 && plot1->canvas()->height() > 10) {
		const double cw = static_cast<double>(plot1->canvas()->width());
		const double ch = static_cast<double>(plot1->canvas()->height());
		const double dLon = std::max(1e-7, lonMax - lonMin);
		const double dM   = std::max(1e-7, mMax - mMin);
		if (cw / dLon < ch / dM) {
			const double targetM = dLon * (ch / cw);
			const double midM = 0.5 * (mMin + mMax);
			mMin = std::clamp(midM - 0.5 * targetM, -180.0, 180.0);
			mMax = std::clamp(midM + 0.5 * targetM, -180.0, 180.0);
		} else {
			const double targetLon = dM * (cw / ch);
			const double midLon = 0.5 * (lonMin + lonMax);
			lonMin = std::clamp(midLon - 0.5 * targetLon, -180.0, 180.0);
			lonMax = std::clamp(midLon + 0.5 * targetLon, -180.0, 180.0);
		}
	}

	const double fittedLatMin = MercatorLatTransform::mercatorDegToLat(mMin);
	const double fittedLatMax = MercatorLatTransform::mercatorDegToLat(mMax);

	const bool doReplot = plot1->autoReplot();
	plot1->setAutoReplot(false);
	plot1->setAxisScale(QwtPlot::xBottom, lonMin, lonMax);
	plot1->setAxisScale(QwtPlot::yLeft, fittedLatMin, fittedLatMax);
	plot1->setAutoReplot(doReplot);
	plot1->replot();
	zoomer->setZoomBase(false);
}

void MapView::addLine(LineItemInfo* line) {
	Figure2::addLine(line);
	maybeTopView = true;
	fitToLines();
}

void MapView::removeLine(LineItemInfo* line) {
	Figure2::removeLine(line);
	fitToLines();
}

void MapView::onResetLayout() {
	fitToLines();
}

int MapView::savePng(const std::string& filename) {
	plot1->replot();
	if (OsmTileManager::instance().hasPendingRequests()) {
		OsmTileManager::instance().waitForPending(1500);
		plot1->replot();
	}
	return Figure2::savePng(filename);
}
