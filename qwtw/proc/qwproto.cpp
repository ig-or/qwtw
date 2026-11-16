#include "xstdef.h"
#include "qwproto.h"
#include "qwproc.h"
#include "qworker.h"

#include <QApplication>
#include <QMetaObject>
#include <algorithm>
#include <cstring>
#include <functional>

QProcInterface::QProcInterface(QWorker& worker_, QApplication& app_)
	: worker(worker_), app(app_) {}

QProcInterface::~QProcInterface() {
	stop();
}

bool QProcInterface::runningAlready() {
	return checkProcRunning() == 1;
}

bool QProcInterface::start() {
	xmprintf(3, "QProcInterface::start() starting\n");
	if (started_.load()) {
		xmprintf(3, "\t - already started\n");
		return true;
	}

	std::string ep = qwipc::endpointPath();
	if (!listener_.listen(ep)) {
		xmprintf(0, "QProcInterface::start(): cannot listen on %s\n", ep.c_str());
		return false;
	}

	worker.qwtSetPickerCallback([this](const CBPickerInfo& cbi) { onCB(cbi); });
	worker.qwtSetClipCallback([this](double t1, double t2, int cg) { onClip(t1, t2, cg); });

	stopping_.store(false);
	started_.store(true);
	acceptThread_ = std::thread(&QProcInterface::acceptLoop, this);
	xmprintf(2, "QProcInterface::start() listening on %s\n", ep.c_str());
	return true;
}

void QProcInterface::stop() {
	if (!started_.exchange(false)) {
		return;
	}
	xmprintf(3, "QProcInterface::stop()\n");
	stopping_.store(true);
	listener_.close();

	{
		std::lock_guard<std::mutex> lock(subscribersMutex_);
		for (auto& sub : subscribers_) {
			if (sub) {
				sub->close();
			}
		}
		subscribers_.clear();
	}

	std::vector<ClientSlot> slotsToJoin;
	{
		std::lock_guard<std::mutex> lock(clientThreadsMutex_);
		for (auto& s : clientSlots_) {
			if (s.stream) {
				s.stream->close();
			}
		}
		slotsToJoin.swap(clientSlots_);
	}

	if (acceptThread_.joinable()) {
		acceptThread_.join();
	}

	std::thread::id selfId = std::this_thread::get_id();
	for (auto& s : slotsToJoin) {
		if (s.worker.joinable()) {
			if (s.worker.get_id() == selfId) {
				s.worker.detach();
			} else {
				s.worker.join();
			}
		}
	}
	xmprintf(3, "QProcInterface::stop() finished\n");
}

void QProcInterface::acceptLoop() {
	using namespace qwipc;
	while (!stopping_.load()) {
		Stream client = listener_.accept();
		if (!client.valid()) {
			if (stopping_.load()) {
				break;
			}
			continue;
		}

		IpcHello hello{};
		if (!client.readAll(&hello, sizeof(hello)) ||
		    hello.magic != kHelloMagic ||
		    hello.version != kProtoVer) {
			client.close();
			continue;
		}

		if (hello.role == RolePing) {
			uint32_t ack = kPingAck;
			client.writeAll(&ack, sizeof(ack));
			client.close();
			continue;
		}

		if (hello.role == RoleCallback) {
			auto sub = std::make_shared<Stream>(std::move(client));
			std::lock_guard<std::mutex> lock(subscribersMutex_);
			subscribers_.push_back(std::move(sub));
			continue;
		}

		if (hello.role == RoleCommand) {
			auto streamPtr = std::make_shared<Stream>(std::move(client));
			std::lock_guard<std::mutex> lock(clientThreadsMutex_);
			// Reap finished slots periodically
			clientSlots_.erase(
				std::remove_if(clientSlots_.begin(), clientSlots_.end(),
					[](ClientSlot& cs) {
						if (cs.stream && !cs.stream->valid() && cs.worker.joinable()) {
							cs.worker.join();
							return true;
						}
						return false;
					}),
				clientSlots_.end());

			ClientSlot slot;
			slot.stream = streamPtr;
			slot.worker = std::thread(&QProcInterface::handleCommandClient, this, streamPtr);
			clientSlots_.push_back(std::move(slot));
			continue;
		}

		client.close();
	}
}

void QProcInterface::handleCommandClient(std::shared_ptr<qwipc::Stream> stream) {
	using namespace qwipc;
	std::vector<double> payloadBuf;

	while (!stopping_.load() && stream->valid()) {
		CmdRequest req{};
		if (!stream->readAll(&req, sizeof(req))) {
			break;
		}
		if (req.magic != kCmdMagic) {
			xmprintf(0, "QProcInterface: bad command magic 0x%x\n", req.magic);
			break;
		}

		// Ensure null-termination on fixed buffers
		req.style[sizeof(req.style) - 1] = '\0';
		req.name[sizeof(req.name) - 1] = '\0';

		constexpr uint64_t kMaxPayloadBytes = 2ULL * 1024ULL * 1024ULL * 1024ULL; // 2 GB sanity cap
		if (req.payloadBytes > kMaxPayloadBytes || (req.payloadBytes % sizeof(double)) != 0) {
			xmprintf(0, "QProcInterface: invalid payloadBytes=%llu\n",
				static_cast<unsigned long long>(req.payloadBytes));
			break;
		}

		size_t numDoubles = static_cast<size_t>(req.payloadBytes / sizeof(double));
		payloadBuf.resize(numDoubles);
		if (req.payloadBytes > 0) {
			if (!stream->readAll(payloadBuf.data(), static_cast<size_t>(req.payloadBytes))) {
				break;
			}
		}

		CmdReply rep{};
		rep.magic = kReplyMagic;
		bool requestQuit = false;
		{
			// Once quit is posted, no other client may queue a blocking call to the GUI thread:
			// the event loop would stop before running it and that thread could never be joined.
			std::lock_guard<std::mutex> execLock(cmdExecMutex_);
			if (stopping_.load()) {
				break;
			}
			executeCommand(req, payloadBuf, rep, requestQuit);
			if (requestQuit) {
				stopping_.store(true);
				listener_.close();
				QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
			}
		}

		if (!stream->writeAll(&rep, sizeof(rep)) || requestQuit) {
			break;
		}
	}

	stream->close();
}

void QProcInterface::onCB(const CBPickerInfo& cbi_) {
	qwipc::ServerEvent ev{};
	ev.magic = qwipc::kEventMagic;
	ev.eventType = qwipc::EvPicker;
	ev.picker = cbi_;
	broadcastEvent(ev);
}

void QProcInterface::onClip(double t1, double t2, int clipGroup) {
	qwipc::ServerEvent ev{};
	ev.magic = qwipc::kEventMagic;
	ev.eventType = qwipc::EvClip;
	ev.clipT1 = t1;
	ev.clipT2 = t2;
	ev.clipGroup = clipGroup;
	broadcastEvent(ev);
}

void QProcInterface::broadcastEvent(const qwipc::ServerEvent& ev) {
	std::lock_guard<std::mutex> lock(subscribersMutex_);
	for (auto it = subscribers_.begin(); it != subscribers_.end();) {
		if (!(*it) || !(*it)->valid() || !(*it)->writeAll(&ev, sizeof(ev))) {
			if (*it) {
				(*it)->close();
			}
			it = subscribers_.erase(it);
		} else {
			++it;
		}
	}
}

void QProcInterface::executeCommand(
	const qwipc::CmdRequest& req,
	const std::vector<double>& payload,
	qwipc::CmdReply& rep,
	bool& requestQuit) {
	using namespace qwipc;
	xmprintf(3, "QProcInterface::executeCommand cmd=%d\n", req.cmd);

	auto sliceArray = [&](size_t& cursor, size_t count) -> double* {
		if (cursor + count > payload.size()) {
			return nullptr;
		}
		// QWorker methods take non-const double* (legacy signature) and copy synchronously
		double* ptr = const_cast<double*>(payload.data() + cursor);
		cursor += count;
		return ptr;
	};

	switch (req.cmd) {
		case CmdHeader::exit:
			xmprintf(2, "QProcInterface: received exit command\n");
			requestQuit = true;
			rep.test = 0;
			break;

		case CmdHeader::qMW:
			worker.qwtshowmw();
			break;

		case CmdHeader::qFigure:
			rep.test = worker.qwtfigure(req.test, req.flags);
			break;

		case CmdHeader::qSpectrogram:
			rep.test = worker.qwtSpectrogram(req.test, req.flags);
			break;

		case CmdHeader::qService:
			rep.test = worker.qwtservice(req.test);
			break;

		case CmdHeader::qSetClipGroup:
			worker.qwtSetClipGroup(req.test);
			break;

		case CmdHeader::qRemoveLine:
			worker.qwtRemoveLine(req.test);
			break;

		case CmdHeader::qSetPos: {
			QWndPos pos{};
			pos.x = static_cast<int>(req.xMin);
			pos.y = static_cast<int>(req.yMin);
			pos.w = req.xSize;
			pos.h = req.ySize;
			pos.set = static_cast<int>(req.flags);

			rep.test = worker.qwtSetPos(req.test, pos);
			rep.flags = static_cast<uint32_t>(pos.set);
			rep.xMin = pos.x;
			rep.yMin = pos.y;
			rep.xSize = pos.w;
			rep.ySize = pos.h;
			break;
		}

		case CmdHeader::qSavePng: {
			char filenameBuf[CmdHeader::nameSize];
			std::memcpy(filenameBuf, req.name, sizeof(filenameBuf));
			filenameBuf[sizeof(filenameBuf) - 1] = '\0';
			rep.test = worker.qwtSavePng(req.test, filenameBuf);
			break;
		}

		case CmdHeader::qMap:
			rep.test = worker.mapview(req.test);
			break;

#ifdef USEMATHGL
		case CmdHeader::qMglPlot:
			rep.test = worker.mglPlot(req.test);
			break;

		case CmdHeader::qMglLine: {
			if (req.size > 0) {
				size_t n = static_cast<size_t>(req.size);
				size_t cursor = 0;
				double* x = sliceArray(cursor, n);
				double* y = sliceArray(cursor, n);
				double* z = sliceArray(cursor, n);
				if (x && y && z) {
					worker.mgl_line(static_cast<int>(req.size), x, y, z, req.name, req.style);
				} else {
					rep.status = -1;
				}
			}
			break;
		}

		case CmdHeader::qMglMesh: {
			size_t n = static_cast<size_t>(req.xSize) * static_cast<size_t>(req.ySize);
			size_t cursor = 0;
			double* data = sliceArray(cursor, n);
			if (data && n > 0) {
				worker.mgl_mesh(req.xSize, req.ySize, req.xMin, req.xMax, req.yMin, req.yMax,
					data, req.name, req.style, req.type);
			} else {
				rep.status = -1;
			}
			break;
		}
#endif

		case CmdHeader::qSpectrogramInfo: {
			if (req.xSize <= 0 || req.ySize <= 0) {
				rep.status = -1;
				break;
			}
			size_t n = static_cast<size_t>(req.xSize) * static_cast<size_t>(req.ySize);
			size_t cursor = 0;
			double* z = sliceArray(cursor, n);
			double* t = (req.arrayMask & ArrT) ? sliceArray(cursor, n) : nullptr;
			double* p = (req.arrayMask & ArrP3) ? sliceArray(cursor, n * 3) : nullptr;
			if (!z || ((req.arrayMask & ArrT) && !t) || ((req.arrayMask & ArrP3) && !p)) {
				rep.status = -1;
				break;
			}
			SpectrogramInfo si{
				req.xSize, req.ySize,
				req.xMin, req.xMax,
				req.yMin, req.yMax,
				z, p, t
			};
			worker.spectrogram_info(si);
			break;
		}

		case CmdHeader::qTitle:
			worker.qwttitle(req.name);
			break;

		case CmdHeader::qXlabel:
			worker.qwtxlabel(req.name);
			break;

		case CmdHeader::qYlabel:
			worker.qwtylabel(req.name);
			break;

		case CmdHeader::qClear:
			worker.qwtclear();
			break;

		case CmdHeader::qImpStatus:
			worker.qwtsetimpstatus(req.test);
			break;

		case CmdHeader::qPlot: {
			if (req.size > 0) {
				size_t n = static_cast<size_t>(req.size);
				size_t cursor = 0;
				double* x = sliceArray(cursor, n);
				double* y = sliceArray(cursor, n);
				if (x && y) {
					rep.test = worker.qwtplot(
						x, y, static_cast<int>(req.size),
						req.name, req.style,
						req.lineWidth, static_cast<int>(req.symSize));
				} else {
					rep.status = -1;
					rep.test = -8;
				}
			}
			break;
		}

		case CmdHeader::qPlot2: {
			if (req.size > 0) {
				size_t n = static_cast<size_t>(req.size);
				size_t cursor = 0;
				double* x = sliceArray(cursor, n);
				double* y = sliceArray(cursor, n);
				double* t = sliceArray(cursor, n);
				if (x && y && t) {
					rep.test = worker.qwtplot2(
						x, y, static_cast<int>(req.size),
						req.name, req.style,
						req.lineWidth, static_cast<int>(req.symSize), t);
				} else {
					rep.status = -1;
					rep.test = -8;
				}
			}
			break;
		}

		case CmdHeader::qChangeLine: {
			if (req.size > 0) {
				size_t n = static_cast<size_t>(req.size);
				size_t cursor = 0;
				double* x = sliceArray(cursor, n);
				double* y = sliceArray(cursor, n);
				double* z = (req.arrayMask & ArrZ) ? sliceArray(cursor, n) : nullptr;
				double* t = (req.arrayMask & ArrT) ? sliceArray(cursor, n) : nullptr;
				if (x && y && (!(req.arrayMask & ArrZ) || z) && (!(req.arrayMask & ArrT) || t)) {
					rep.test = worker.qwtChangeLine(req.test, x, y, z, t, static_cast<int>(req.size));
				} else {
					rep.status = -1;
					rep.test = -8;
				}
			}
			break;
		}

		case CmdHeader::qEnableBC: {
			if (req.size > 0) {
				size_t n = static_cast<size_t>(req.size);
				size_t cursor = 0;
				double* x = sliceArray(cursor, n);
				double* y = sliceArray(cursor, n);
				double* z = sliceArray(cursor, n);
				double* t = sliceArray(cursor, n);
				if (x && y && z && t) {
					worker.qwtEnableCoordBroadcast(x, y, z, t, static_cast<int>(req.size));
				}
			}
			break;
		}

		case CmdHeader::qDisableBC:
			worker.qwtDisableCoordBroadcast();
			break;

		case CmdHeader::qSetLogLevel:
			xmPrintLevel = req.test;
			xmprintf(1, "got qSetLogLevel with [%d]\n", req.test);
			break;

		default:
			break;
	}
}
