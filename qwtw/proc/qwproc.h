#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include "qwtypes.h"

struct CmdHeader {
	enum QWCmd {
		nothing = 0,
		exit = 1,
		changeSize,
		changeDataSize,
		qMW,
		qFigure,
		qSpectrogram,
		qSpectrogramInfo,
		qRemoveLine,
		qMap,
		qPlot,
		qPlot2,
		qChangeLine,
		qTitle,
		qXlabel,
		qYlabel,
		qClear,
		qImpStatus,
		qEnableBC,
		qDisableBC,
		qSetLogLevel,
		qMglPlot,
		qMglLine,
		qMglMesh,
		qSetClipGroup,
		qSetUdpCallback,
		qSetPickerCallback,
		qService,
		qSavePng,
		qSetPos,

		qwcmdSize
	};
	enum {
		nameSize = 356
	};
};

namespace qwipc {

constexpr uint32_t kHelloMagic = 0x51575457u; // 'QWTW'
constexpr uint32_t kCmdMagic   = 0x5157434Du; // 'QWCM'
constexpr uint32_t kReplyMagic = 0x51575250u; // 'QWRP'
constexpr uint32_t kEventMagic = 0x51574556u; // 'QWEV'
constexpr uint32_t kPingAck    = 0x51574F4Bu; // 'QWOK'
constexpr uint16_t kProtoVer   = 1;

enum IpcRole : uint16_t {
	RoleCommand  = 1,
	RoleCallback = 2,
	RolePing     = 3
};

enum ArrayFlag : uint32_t {
	ArrNone = 0,
	ArrX    = 1u << 0,
	ArrY    = 1u << 1,
	ArrZ    = 1u << 2,
	ArrT    = 1u << 3,
	ArrData = 1u << 4,
	ArrP3   = 1u << 5
};

enum EventType : uint32_t {
	EvPicker = 1,
	EvClip   = 2
};

#pragma pack(push, 8)

struct IpcHello {
	uint32_t magic   = kHelloMagic;
	uint16_t version = kProtoVer;
	uint16_t role    = RoleCommand;
};

struct CmdRequest {
	uint32_t magic        = kCmdMagic;
	int32_t  cmd          = CmdHeader::nothing;
	int64_t  size         = 0;
	int32_t  lineWidth    = 0;
	int64_t  symSize      = 0;
	int32_t  test         = 0;
	uint32_t flags        = 0;
	int32_t  xSize        = 0;
	int32_t  ySize        = 0;
	int32_t  type         = 0;
	uint32_t arrayMask    = ArrNone;
	double   xMin         = 0.0;
	double   xMax         = 0.0;
	double   yMin         = 0.0;
	double   yMax         = 0.0;
	char     style[32]    = {0};
	char     name[CmdHeader::nameSize] = {0};
	uint64_t payloadBytes = 0;
};

struct CmdReply {
	uint32_t magic  = kReplyMagic;
	int32_t  status = 0;
	int32_t  test   = 0;
	uint32_t flags  = 0;
	int32_t  xSize  = 0;
	int32_t  ySize  = 0;
	double   xMin   = 0.0;
	double   yMin   = 0.0;
};

struct ServerEvent {
	uint32_t     magic     = kEventMagic;
	uint32_t     eventType = 0;
	CBPickerInfo picker;
	double       clipT1    = 0.0;
	double       clipT2    = 0.0;
	int32_t      clipGroup = 0;
	int32_t      reserved  = 0;
};

#pragma pack(pop)

/// Per-user local socket path (/tmp/qwtw-<uid>.sock on Unix, \\.\pipe\qwtw-<user> on Windows).
std::string endpointPath();

/// Bidirectional local IPC byte stream (AF_UNIX socket on POSIX, Named Pipe on Windows).
class Stream {
public:
	Stream() noexcept;
	~Stream();

	Stream(Stream&& other) noexcept;
	Stream& operator=(Stream&& other) noexcept;

	Stream(const Stream&) = delete;
	Stream& operator=(const Stream&) = delete;

	static Stream connect(const std::string& endpoint);

	bool valid() const noexcept;
	bool writeAll(const void* data, size_t bytes);
	bool readAll(void* data, size_t bytes);
	void close() noexcept;

#ifdef WIN32
	explicit Stream(void* handle, bool isServerPipe = false) noexcept;
#else
	explicit Stream(int fd) noexcept;
#endif

private:
#ifdef WIN32
	std::atomic<void*> handle_;
	bool isServerPipe_ = false;
#else
	std::atomic<int> fd_;
#endif
};

/// Single-instance local IPC server listener.
class Listener {
public:
	Listener() noexcept;
	~Listener();

	Listener(const Listener&) = delete;
	Listener& operator=(const Listener&) = delete;

	bool listen(const std::string& endpoint);
	Stream accept();
	void close() noexcept;
	bool isListening() const noexcept;

private:
	std::string endpoint_;
	std::atomic<bool> closing_{false};
#ifdef WIN32
	std::atomic<void*> pendingPipe_;
#else
	std::atomic<int> listenFd_;
#endif
};

} // namespace qwipc

/** get location for all the 'system' files
\param p[in] put location here
\param pSize size of the 'p'
\return false if error, true if all is OK
*/
bool getFolderLocation(char* p, int pSize);

/**
\return 0 if proc is not running; 1 if running; smth else if error
*/
int checkProcRunning();
extern int xmPrintLevel;
int xmprintf(int level, const char* _Format, ...);
