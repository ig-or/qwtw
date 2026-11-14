#include "xstdef.h"
#include "pclient.h"
#include "qwtypes.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef __APPLE__
	#include <dlfcn.h>
	#include <limits.h>
	#include <mach-o/dyld.h>
	#include <spawn.h>
	#include <sys/wait.h>
	#include <unistd.h>
	extern char** environ;
#elif defined(__linux__) || defined(__linux)
	#include <dlfcn.h>
	#include <limits.h>
	#include <spawn.h>
	#include <sys/wait.h>
	#include <unistd.h>
	extern char** environ;
#elif defined(WIN32)
	#include <windows.h>
	const char* getQwtwDllPath();
#endif

#if defined(__APPLE__) || defined(__linux__) || defined(__linux)
static void qwtw_lib_anchor() {}

// Return directory containing this shared library so qwproc can be found next to it.
static std::string getQwtwLibDir() {
	Dl_info info{};
	if (dladdr(reinterpret_cast<void*>(&qwtw_lib_anchor), &info) != 0 && info.dli_fname && info.dli_fname[0]) {
		std::filesystem::path p(info.dli_fname);
		return p.parent_path().string();
	}
	return std::string();
}
#endif

static std::string searchExecutableInPath(const char* procName) {
	const char* pathEnv = std::getenv("PATH");
	if (!pathEnv) {
		return std::string();
	}
#ifdef WIN32
	constexpr char kSep = ';';
#else
	constexpr char kSep = ':';
#endif
	std::stringstream ss(pathEnv);
	std::string entry;
	std::error_code ec;
	while (std::getline(ss, entry, kSep)) {
		if (entry.empty()) {
			continue;
		}
		std::filesystem::path candidate = std::filesystem::path(entry) / procName;
		if (std::filesystem::exists(candidate, ec) && !std::filesystem::is_directory(candidate, ec)) {
#ifndef WIN32
			if (::access(candidate.c_str(), X_OK) != 0) {
				continue;
			}
#endif
			return candidate.string();
		}
	}
	return std::string();
}

// Resolve full path to qwproc executable (current dir, DLL/lib dir, exe dir, then PATH).
static std::string findQwProcPath() {
	namespace fs = std::filesystem;
#ifdef WIN32
	const char* procName = "qwproc.exe";
#else
	const char* procName = "qwproc";
#endif
	std::error_code ec;

	// 1. Current working directory
	fs::path cwd = fs::current_path(ec);
	if (!ec) {
		fs::path pa = cwd / procName;
		xmprintf(2, "\tfirst, check current path; looking at %s ..\n", pa.string().c_str());
		if (fs::exists(pa, ec) && !fs::is_directory(pa, ec)) {
			xmprintf(2, "\tlocated!\n");
			return pa.string();
		}
	}

#ifdef WIN32
	// 2. Directory of the loading DLL
	const char* dllPath = getQwtwDllPath();
	if (dllPath && dllPath[0] != '\0') {
		fs::path pa = fs::path(dllPath).parent_path() / procName;
		xmprintf(2, "\tnext, check near the DLL; looking at %s ..\n", pa.string().c_str());
		if (fs::exists(pa, ec) && !fs::is_directory(pa, ec)) {
			xmprintf(2, "\tlocated!\n");
			return pa.string();
		}
	}
#elif defined(__APPLE__) || defined(__linux__) || defined(__linux)
	// 2. Directory of this shared library
	{
		std::string libDir = getQwtwLibDir();
		if (!libDir.empty()) {
			fs::path pa = fs::path(libDir) / procName;
			xmprintf(2, "\tcheck library dir; looking at %s\n", pa.string().c_str());
			if (fs::exists(pa, ec) && !fs::is_directory(pa, ec)) {
				xmprintf(2, "\tlocated next to library\n");
				return pa.string();
			}
		}
	}
#endif

#if defined(__APPLE__)
	// 3. Directory of the main executable (macOS)
	{
		char result[PATH_MAX] = {0};
		uint32_t bufsize = sizeof(result);
		if (_NSGetExecutablePath(result, &bufsize) == 0) {
			fs::path pa = fs::path(result).parent_path() / procName;
			xmprintf(2, "\tlooking at %s\n", pa.string().c_str());
			if (fs::exists(pa, ec) && !fs::is_directory(pa, ec)) {
				xmprintf(2, "\tgot exe in %s\n", pa.string().c_str());
				return pa.string();
			}
			xmprintf(2, "\t\t .. nothing found\n");
		}
	}
#elif defined(__linux__) || defined(__linux)
	// 3. Directory of the main executable (Linux)
	{
		char result[PATH_MAX] = {0};
		ssize_t count = ::readlink("/proc/self/exe", result, sizeof(result) - 1);
		if (count > 0) {
			result[count] = '\0';
			fs::path pa = fs::path(result).parent_path() / procName;
			xmprintf(2, "\tlooking at %s\n", pa.string().c_str());
			if (fs::exists(pa, ec) && !fs::is_directory(pa, ec)) {
				xmprintf(2, "\tgot exe in %s\n", pa.string().c_str());
				return pa.string();
			}
			xmprintf(2, "\t\t .. nothing found\n");
		}
	}
#endif

	// 4. Search PATH
	xmprintf(2, "\tnow looking in PATH\n");
	std::string found = searchExecutableInPath(procName);
	if (!found.empty()) {
		xmprintf(2, "got qwproc in [%s]\n", found.c_str());
		return found;
	}

	xmprintf(0, "cannot find %s in PATH\n", procName);
	return std::string();
}

static bool spawnProcessDetached(const std::string& exePath, const std::vector<std::string>& args) {
#ifdef WIN32
	std::string cmdLine = "\"" + exePath + "\"";
	for (const auto& a : args) {
		cmdLine += " \"";
		cmdLine += a;
		cmdLine += "\"";
	}
	std::vector<char> mutableCmd(cmdLine.begin(), cmdLine.end());
	mutableCmd.push_back('\0');

	STARTUPINFOA si{};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi{};
	BOOL ok = CreateProcessA(
		exePath.c_str(),
		mutableCmd.data(),
		NULL,
		NULL,
		FALSE,
		DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP,
		NULL,
		NULL,
		&si,
		&pi);
	if (!ok) {
		return false;
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return true;
#else
	std::vector<std::string> fullArgs;
	fullArgs.reserve(args.size() + 1);
	fullArgs.push_back(exePath);
	for (const auto& a : args) {
		fullArgs.push_back(a);
	}
	std::vector<char*> argv;
	argv.reserve(fullArgs.size() + 1);
	for (auto& s : fullArgs) {
		argv.push_back(s.data());
	}
	argv.push_back(nullptr);

	posix_spawnattr_t attr;
	if (posix_spawnattr_init(&attr) != 0) {
		return false;
	}
#ifdef POSIX_SPAWN_SETSID
	if (args.size() >= 2 && args[args.size() - 2] == "--debug" && args.back() != "0") {
		posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSID);
	}
#endif
	pid_t pid = -1;
	int rc = posix_spawn(&pid, exePath.c_str(), nullptr, &attr, argv.data(), environ);
	posix_spawnattr_destroy(&attr);
	if (rc != 0) {
		return false;
	}
	// If debug == 0, qwproc daemonizes immediately by exiting its first parent; reap if quick.
	int wstatus = 0;
	::waitpid(pid, &wstatus, WNOHANG);
	return true;
#endif
}

SHMTest::SHMTest() : status(5) {}

SHMTest::~SHMTest() {
	onClose();
}

bool SHMTest::connectChannels() {
	using namespace qwipc;
	std::string ep = endpointPath();

	Stream cmd = Stream::connect(ep);
	if (!cmd.valid()) {
		return false;
	}
	IpcHello cmdHello{kHelloMagic, kProtoVer, RoleCommand};
	if (!cmd.writeAll(&cmdHello, sizeof(cmdHello))) {
		return false;
	}

	Stream cb = Stream::connect(ep);
	if (!cb.valid()) {
		return false;
	}
	IpcHello cbHello{kHelloMagic, kProtoVer, RoleCallback};
	if (!cb.writeAll(&cbHello, sizeof(cbHello))) {
		return false;
	}

	cmdStream_ = std::move(cmd);
	cbStream_ = std::move(cb);
	return true;
}

bool SHMTest::sendTransaction(
	qwipc::CmdRequest& req,
	const std::vector<PayloadChunk>& chunks,
	qwipc::CmdReply& rep) {
	std::lock_guard<std::mutex> lock(ioMutex_);
	if (!cmdStream_.valid()) {
		status = 2;
		return false;
	}

	uint64_t totalBytes = 0;
	for (const auto& chunk : chunks) {
		if (chunk.first != nullptr && chunk.second > 0) {
			totalBytes += static_cast<uint64_t>(chunk.second) * sizeof(double);
		}
	}
	req.magic = qwipc::kCmdMagic;
	req.payloadBytes = totalBytes;

	if (!cmdStream_.writeAll(&req, sizeof(req))) {
		cmdStream_.close();
		status = 2;
		return false;
	}

	for (const auto& chunk : chunks) {
		if (chunk.first != nullptr && chunk.second > 0) {
			if (!cmdStream_.writeAll(chunk.first, chunk.second * sizeof(double))) {
				cmdStream_.close();
				status = 2;
				return false;
			}
		}
	}

	if (!cmdStream_.readAll(&rep, sizeof(rep)) || rep.magic != qwipc::kReplyMagic) {
		cmdStream_.close();
		status = 2;
		return false;
	}
	return true;
}

int SHMTest::startProc(int level) {
	xmprintf(2, "\nstarting proc..\n");
	std::string qwProcPath = findQwProcPath();
	if (qwProcPath.empty()) {
		return 2;
	}

	std::vector<std::string> args;
	args.push_back("--debug");
	args.push_back(std::to_string(level));

	xmprintf(2, "@starting %s --debug %d\n", qwProcPath.c_str(), level);
	if (!spawnProcessDetached(qwProcPath, args)) {
		xmprintf(0, "cannot start process from (%s)\n", qwProcPath.c_str());
		return 1;
	}
	return 0;
}

int SHMTest::testInit(int level) {
	using namespace std::chrono_literals;
	xmprintf(2, "starting SHMTest::testInit()\n");

	if (status == 0 && cmdStream_.valid()) {
		qsetloglevel(level);
		return 0;
	}

	onClose();

	bool connected = connectChannels();
	if (!connected) {
		int sr = startProc(level);
		if (sr != 0) {
			status = 1;
			return sr;
		}

		// Poll every 50ms up to 6s — typically connects within 100–250ms instead of sleeping 550ms+ upfront
		for (int attempt = 0; attempt < 120; ++attempt) {
			std::this_thread::sleep_for(50ms);
			if (connectChannels()) {
				connected = true;
				xmprintf(2, "SHMTest::testInit() connected after %d polls\n", attempt + 1);
				break;
			}
		}
	}

	if (!connected) {
		status = 1;
		xmprintf(0, "ERROR: cannot start or connect to qwproc\n");
		return 1;
	}

	status = 0;
	qsetloglevel(level);

	stopCbThread_.store(false);
	cbThread_ = std::thread(&SHMTest::cbThreadF, this);

	xmprintf(2, "SHMTest::testInit() finished\n");
	return 0;
}

void SHMTest::onClose() {
	xmprintf(9, "SHMTest::onClose() starting\n");
	stopCbThread_.store(true);
	cbStream_.close();
	if (cbThread_.joinable()) {
		if (cbThread_.get_id() == std::this_thread::get_id()) {
			cbThread_.detach();
		} else {
			cbThread_.join();
		}
	}
	{
		std::lock_guard<std::mutex> lock(ioMutex_);
		cmdStream_.close();
	}
	if (status == 0) {
		status = 5;
	}
	xmprintf(9, "SHMTest::onClose() finished\n");
}

void SHMTest::stopQt() {
	if (status != 0) {
		return;
	}
	qwipc::CmdRequest req{};
	req.cmd = CmdHeader::exit;
	qwipc::CmdReply rep{};
	sendTransaction(req, {}, rep);

	onClose();
	status = 4;
	xmprintf(3, "SHMTest::stopQt() done\n");
}

void SHMTest::cbThreadF() {
	using namespace qwipc;
	while (!stopCbThread_.load() && cbStream_.valid()) {
		ServerEvent ev{};
		if (!cbStream_.readAll(&ev, sizeof(ev))) {
			break;
		}
		if (stopCbThread_.load() || ev.magic != kEventMagic) {
			break;
		}

		OnPCallback pickerCb = nullptr;
		OnClipCallback clipCb = nullptr;
		CBTest_1 t1Cb = nullptr;
		CBTest_2 t2Cb = nullptr;
		{
			std::lock_guard<std::mutex> lock(cbHandlerMutex_);
			pickerCb = pCallback;
			clipCb = pClipCallback;
			t1Cb = cbTest1;
			t2Cb = cbTest2;
		}

		if (ev.eventType == EvPicker) {
			ev.picker.label[CBPickerInfo::lSize - 1] = '\0';
			if (pickerCb) {
				pickerCb(
					ev.picker.plotID,
					ev.picker.lineID,
					ev.picker.index,
					ev.picker.xx,
					ev.picker.yy,
					ev.picker.x,
					ev.picker.y,
					ev.picker.z,
					ev.picker.time,
					ev.picker.label);
			}
			if (t1Cb) {
				t1Cb();
			}
			if (t2Cb) {
				t2Cb(ev.picker.index);
			}
		} else if (ev.eventType == EvClip) {
			if (clipCb) {
				clipCb(ev.clipT1, ev.clipT2, ev.clipGroup);
			}
		}
	}
}

void SHMTest::setCB(OnPCallback cb) {
	std::lock_guard<std::mutex> lock(cbHandlerMutex_);
	pCallback = cb;
}

void SHMTest::setClipCallback(OnClipCallback cb) {
	std::lock_guard<std::mutex> lock(cbHandlerMutex_);
	pClipCallback = cb;
}

void SHMTest::setCBTest1(CBTest_1 cb) {
	std::lock_guard<std::mutex> lock(cbHandlerMutex_);
	cbTest1 = cb;
}

void SHMTest::setCBTest2(CBTest_2 cb) {
	std::lock_guard<std::mutex> lock(cbHandlerMutex_);
	cbTest2 = cb;
}

int SHMTest::sendCommand(CmdHeader::QWCmd cmd, const char* text, int test) {
	if (status != 0) {
		return 0;
	}
	qwipc::CmdRequest req{};
	req.cmd = cmd;
	req.test = test;
	if (text != nullptr) {
		std::snprintf(req.name, sizeof(req.name), "%s", text);
	}
	qwipc::CmdReply rep{};
	if (!sendTransaction(req, {}, rep)) {
		return -1;
	}
	return rep.test;
}

int SHMTest::sendCommand(CmdHeader::QWCmd cmd, int v, unsigned int flags) {
	if (status != 0) {
		return 0;
	}
	qwipc::CmdRequest req{};
	req.cmd = cmd;
	req.test = v;
	req.flags = flags;
	qwipc::CmdReply rep{};
	if (!sendTransaction(req, {}, rep)) {
		return -1;
	}
	return rep.test;
}

int SHMTest::qwtmap(int n) {
	if (status != 0) return 0;
	return sendCommand(CmdHeader::qMap, n);
}

int SHMTest::qwtservice(int x) {
	if (status != 0) return 0;
	return sendCommand(CmdHeader::qService, x);
}

int SHMTest::qwtfigure(int n, unsigned int flags) {
	if (status != 0) return 0;
	return sendCommand(CmdHeader::qFigure, n, flags);
}

int SHMTest::qwtspectrogram(int n, unsigned int flags) {
	if (status != 0) return 0;
	return sendCommand(CmdHeader::qSpectrogram, n, flags);
}

int SHMTest::spectrogram_info(const SpectrogramInfo& info) {
	if (status != 0) return 0;
	if (info.nx <= 0 || info.ny <= 0 || info.z == nullptr) {
		return -1;
	}

	size_t n = static_cast<size_t>(info.nx) * static_cast<size_t>(info.ny);
	qwipc::CmdRequest req{};
	req.cmd = CmdHeader::qSpectrogramInfo;
	req.size = static_cast<int64_t>(n);
	req.xSize = info.nx;
	req.ySize = info.ny;
	req.xMin = info.xmin;
	req.xMax = info.xmax;
	req.yMin = info.ymin;
	req.yMax = info.ymax;
	req.arrayMask = qwipc::ArrData;

	std::vector<PayloadChunk> chunks;
	chunks.emplace_back(info.z, n);

	if (info.t != nullptr) {
		req.flags |= 1u;
		req.arrayMask |= qwipc::ArrT;
		chunks.emplace_back(info.t, n);
	}
	if (info.p != nullptr) {
		req.flags |= 2u;
		req.arrayMask |= qwipc::ArrP3;
		chunks.emplace_back(info.p, n * 3);
	}

	qwipc::CmdReply rep{};
	if (!sendTransaction(req, chunks, rep)) {
		return -1;
	}
	return rep.status;
}

void SHMTest::qwtClipGroup(int gr) {
	if (status != 0) return;
	sendCommand(CmdHeader::qSetClipGroup, gr);
}

void SHMTest::qwtsetimpstatus(int impStatus) {
	if (status != 0) return;
	sendCommand(CmdHeader::qImpStatus, impStatus);
}

void SHMTest::qwtshowmw() {
	if (status != 0) return;
	sendCommand(CmdHeader::qMW);
}

void SHMTest::qwttitle(const char* s) {
	if (status != 0) return;
	sendCommand(CmdHeader::qTitle, s);
}

void SHMTest::qwtxlabel(const char* s) {
	if (status != 0) return;
	sendCommand(CmdHeader::qXlabel, s);
}

void SHMTest::qwtylabel(const char* s) {
	if (status != 0) return;
	sendCommand(CmdHeader::qYlabel, s);
}

void SHMTest::qwtclear() {
	if (status != 0) return;
	sendCommand(CmdHeader::qClear);
}

void SHMTest::qsetloglevel(int level) {
	if (status != 0) return;
	sendCommand(CmdHeader::qSetLogLevel, level);
}

#ifdef ENABLE_UDP_SYNC
void SHMTest::qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size) {
	if (status != 0 || size <= 0 || !x || !y || !z || !time) return;
	size_t n = static_cast<size_t>(size);
	qwipc::CmdRequest req{};
	req.cmd = CmdHeader::qEnableBC;
	req.size = size;
	req.arrayMask = qwipc::ArrX | qwipc::ArrY | qwipc::ArrZ | qwipc::ArrT;

	qwipc::CmdReply rep{};
	sendTransaction(req, {{x, n}, {y, n}, {z, n}, {time, n}}, rep);
}

void SHMTest::qwtDisableCoordBroadcast() {
	if (status != 0) return;
	sendCommand(CmdHeader::qDisableBC);
}
#endif

int SHMTest::qwtplot(double* x, double* y, int size, const char* name, const char* style,
	int lineWidth, int symSize) {
	return qwtplot2(x, y, size, name, style, lineWidth, symSize, nullptr);
}

int SHMTest::qwtplot2(double* x, double* y, int size, const char* name, const char* style,
	int lineWidth, int symSize, double* time) {
	if (status != 0) return -7;
	if (size <= 0 || x == nullptr || y == nullptr) return -8;

	size_t n = static_cast<size_t>(size);
	qwipc::CmdRequest req{};
	req.lineWidth = lineWidth;
	req.symSize = symSize;
	req.size = size;
	if (style) {
		std::snprintf(req.style, sizeof(req.style), "%s", style);
	}
	if (name) {
		std::snprintf(req.name, sizeof(req.name), "%s", name);
	}

	std::vector<PayloadChunk> chunks;
	chunks.emplace_back(x, n);
	chunks.emplace_back(y, n);
	req.arrayMask = qwipc::ArrX | qwipc::ArrY;

	if (time != nullptr) {
		req.cmd = CmdHeader::qPlot2;
		req.arrayMask |= qwipc::ArrT;
		chunks.emplace_back(time, n);
	} else {
		req.cmd = CmdHeader::qPlot;
	}

	qwipc::CmdReply rep{};
	if (!sendTransaction(req, chunks, rep)) {
		return -8;
	}
	return rep.test;
}

void SHMTest::qwtremove(int id) {
	if (status != 0) return;
	sendCommand(CmdHeader::qRemoveLine, id);
}

int SHMTest::qwtsave_png(int id, char* filename) {
	if (status != 0) return -7;
	return sendCommand(CmdHeader::qSavePng, filename, id);
}

int SHMTest::qwtsetpos(int key, QWndPos& pos) {
	if (status != 0) return -7;

	qwipc::CmdRequest req{};
	req.cmd = CmdHeader::qSetPos;
	req.test = key;
	req.flags = static_cast<uint32_t>(pos.set);
	req.xMin = pos.x;
	req.yMin = pos.y;
	req.xSize = pos.w;
	req.ySize = pos.h;

	qwipc::CmdReply rep{};
	if (!sendTransaction(req, {}, rep)) {
		return -7;
	}

	pos.x = static_cast<int>(rep.xMin);
	pos.y = static_cast<int>(rep.yMin);
	pos.w = rep.xSize;
	pos.h = rep.ySize;
	pos.set = static_cast<int>(rep.flags);
	return rep.test;
}

int SHMTest::qwtchange(int id, double* x, double* y, double* z, double* time, int size) {
	if (status != 0) return -7;
	if (size <= 0 || x == nullptr || y == nullptr) return -8;

	size_t n = static_cast<size_t>(size);
	qwipc::CmdRequest req{};
	req.cmd = CmdHeader::qChangeLine;
	req.test = id;
	req.size = size;
	req.arrayMask = qwipc::ArrX | qwipc::ArrY;

	std::vector<PayloadChunk> chunks;
	chunks.emplace_back(x, n);
	chunks.emplace_back(y, n);

	if (z != nullptr) {
		req.arrayMask |= qwipc::ArrZ;
		chunks.emplace_back(z, n);
	}
	if (time != nullptr) {
		req.arrayMask |= qwipc::ArrT;
		chunks.emplace_back(time, n);
	}

	qwipc::CmdReply rep{};
	if (!sendTransaction(req, chunks, rep)) {
		return -8;
	}
	return rep.test;
}

#ifdef USEMATHGL
int SHMTest::qwtmgl(int n) {
	if (status != 0) return 0;
	return sendCommand(CmdHeader::qMglPlot, n);
}

void SHMTest::qwtmgl_line(int size, double* x, double* y, double* z, const char* name, const char* style) {
	if (status != 0 || size <= 0 || !x || !y || !z) return;
	size_t n = static_cast<size_t>(size);

	qwipc::CmdRequest req{};
	req.cmd = CmdHeader::qMglLine;
	req.size = size;
	req.arrayMask = qwipc::ArrX | qwipc::ArrY | qwipc::ArrZ;
	if (style) {
		std::snprintf(req.style, sizeof(req.style), "%s", style);
	}
	if (name) {
		std::snprintf(req.name, sizeof(req.name), "%s", name);
	}

	qwipc::CmdReply rep{};
	sendTransaction(req, {{x, n}, {y, n}, {z, n}}, rep);
}

void SHMTest::qwtmgl_mesh(const MeshInfo& info) {
	if (status != 0 || info.xSize <= 0 || info.ySize <= 0 || !info.data) return;
	size_t n = static_cast<size_t>(info.xSize) * static_cast<size_t>(info.ySize);

	qwipc::CmdRequest req{};
	req.cmd = CmdHeader::qMglMesh;
	req.size = static_cast<int64_t>(n);
	req.xSize = info.xSize;
	req.ySize = info.ySize;
	req.xMin = info.xMin;
	req.xMax = info.xMax;
	req.yMin = info.yMin;
	req.yMax = info.yMax;
	req.type = static_cast<int>(info.sd);
	req.arrayMask = qwipc::ArrData;
	std::snprintf(req.style, sizeof(req.style), "%s", info.style.c_str());
	std::snprintf(req.name, sizeof(req.name), "%s", info.name.c_str());

	qwipc::CmdReply rep{};
	sendTransaction(req, {{info.data, n}}, rep);
}
#endif
