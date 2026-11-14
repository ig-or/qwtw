#pragma once

#include "figure2.h"
#include <qwt_transform.h>

class OsmTileItem;

/**
 * Web Mercator (EPSG:3857) latitude transform for Qwt's Y axis.
 *
 * Maps WGS-84 latitude in degrees [-85.05112878, +85.05112878] to Mercator
 * latitude-equivalent degrees [-180, +180], where 1 unit has the exact same
 * Web Mercator metric scale as 1 degree of longitude on the X axis.
 */
class MercatorLatTransform : public QwtTransform {
public:
	static constexpr double kMaxLat = 85.05112878;

	MercatorLatTransform() = default;
	~MercatorLatTransform() override = default;

	double bounded(double value) const override;
	double transform(double value) const override;
	double invTransform(double value) const override;
	QwtTransform* copy() const override;

	static double latToMercatorDeg(double latDeg);
	static double mercatorDegToLat(double mercDeg);
};

/**
 * Native Qt6 + Qwt OpenStreetMap slippy-map plot window (`qwtmap` / `qfmap`).
 *
 * Inherits all interactive capabilities of `Figure2` (synchronized time markers,
 * picker callbacks, UDP broadcast, zoom/pan, clip groups, `qremove`, `qchange`,
 * permanent markers, and `qsavepng`), while rendering cached Web Mercator tiles
 * underneath the plot curves.
 */
class MapView : public Figure2 {
	Q_OBJECT
public:
	MapView(const std::string& key_, XQPlots* pf_, QWidget* parent, unsigned int flags_ = 0);
	~MapView() override;

	void addLine(LineItemInfo* line) override;
	void removeLine(LineItemInfo* line) override;
	void onResetLayout() override;
	int savePng(const std::string& filename) override;

private:
	OsmTileItem* tileItem_ = nullptr;
	void fitToLines();
};
