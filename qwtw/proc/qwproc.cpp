#include "xstdef.h"
#include "qwproc.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#ifdef WIN32
	#include <windows.h>
	#include <shlobj.h>
#else
	#include <fcntl.h>
	#include <poll.h>
	#include <pwd.h>
	#include <sys/socket.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/un.h>
	#include <unistd.h>
#endif

namespace qwipc {

std::string endpointPath() {
#ifdef WIN32
	char user[128] = {0};
	DWORD len = sizeof(user) - 1;
	if (!GetUserNameA(user, &len) || user[0] == '\0') {
		std::snprintf(user, sizeof(user), "default");
	}
	for (char* c = user; *c != '\0'; ++c) {
		if (*c == '\\' || *c == '/' || *c == ' ') {
			*c = '_';
		}
	}
	return std::string("\\\\.\\pipe\\qwtw-") + user;
#else
	unsigned long uid = static_cast<unsigned long>(::getuid());
	return "/tmp/qwtw-" + std::to_string(uid) + ".sock";
#endif
}

#ifdef WIN32

Stream::Stream() noexcept : handle_(INVALID_HANDLE_VALUE), isServerPipe_(false) {}

Stream::Stream(void* handle, bool isServerPipe) noexcept
	: handle_(handle), isServerPipe_(isServerPipe) {}

Stream::~Stream() {
	close();
}

Stream::Stream(Stream&& other) noexcept
	: handle_(other.handle_.exchange(INVALID_HANDLE_VALUE)),
	  isServerPipe_(other.isServerPipe_) {
	other.isServerPipe_ = false;
}

Stream& Stream::operator=(Stream&& other) noexcept {
	if (this != &other) {
		close();
		handle_.store(other.handle_.exchange(INVALID_HANDLE_VALUE));
		isServerPipe_ = other.isServerPipe_;
		other.isServerPipe_ = false;
	}
	return *this;
}

Stream Stream::connect(const std::string& endpoint) {
	for (int attempt = 0; attempt < 20; ++attempt) {
		HANDLE h = CreateFileA(
			endpoint.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);
		if (h != INVALID_HANDLE_VALUE) {
			return Stream(h, false);
		}
		DWORD err = GetLastError();
		if (err == ERROR_PIPE_BUSY) {
			if (!WaitNamedPipeA(endpoint.c_str(), 50)) {
				Sleep(15);
			}
		} else {
			break;
		}
	}
	return Stream();
}

bool Stream::valid() const noexcept {
	return handle_.load() != INVALID_HANDLE_VALUE;
}

namespace {

struct ScopedWinEvent {
	HANDLE h = NULL;
	ScopedWinEvent() noexcept : h(CreateEventW(NULL, TRUE, FALSE, NULL)) {}
	~ScopedWinEvent() {
		if (h != NULL) {
			CloseHandle(h);
		}
	}
	ScopedWinEvent(const ScopedWinEvent&) = delete;
	ScopedWinEvent& operator=(const ScopedWinEvent&) = delete;
};

} // namespace

bool Stream::writeAll(const void* data, size_t bytes) {
	if (bytes == 0) {
		return true;
	}
	ScopedWinEvent ev;
	if (ev.h == NULL) {
		return false;
	}

	const auto* ptr = static_cast<const unsigned char*>(data);
	size_t remaining = bytes;
	while (remaining > 0) {
		HANDLE h = static_cast<HANDLE>(handle_.load());
		if (h == INVALID_HANDLE_VALUE) {
			return false;
		}
		ResetEvent(ev.h);
		OVERLAPPED ov{};
		ov.hEvent = ev.h;

		DWORD chunk = static_cast<DWORD>(std::min<size_t>(remaining, 64u * 1024u * 1024u));
		DWORD written = 0;
		BOOL ok = WriteFile(h, ptr, chunk, &written, &ov);
		if (!ok) {
			if (GetLastError() != ERROR_IO_PENDING) {
				return false;
			}
			if (!GetOverlappedResult(h, &ov, &written, TRUE)) {
				return false;
			}
		}
		if (written == 0) {
			return false;
		}
		ptr += written;
		remaining -= written;
	}
	return true;
}

bool Stream::readAll(void* data, size_t bytes) {
	if (bytes == 0) {
		return true;
	}
	ScopedWinEvent ev;
	if (ev.h == NULL) {
		return false;
	}

	auto* ptr = static_cast<unsigned char*>(data);
	size_t remaining = bytes;
	while (remaining > 0) {
		HANDLE h = static_cast<HANDLE>(handle_.load());
		if (h == INVALID_HANDLE_VALUE) {
			return false;
		}
		ResetEvent(ev.h);
		OVERLAPPED ov{};
		ov.hEvent = ev.h;

		DWORD chunk = static_cast<DWORD>(std::min<size_t>(remaining, 64u * 1024u * 1024u));
		DWORD got = 0;
		BOOL ok = ReadFile(h, ptr, chunk, &got, &ov);
		if (!ok) {
			if (GetLastError() != ERROR_IO_PENDING) {
				return false;
			}
			if (!GetOverlappedResult(h, &ov, &got, TRUE)) {
				return false;
			}
		}
		if (got == 0) {
			return false;
		}
		ptr += got;
		remaining -= got;
	}
	return true;
}

void Stream::close() noexcept {
	HANDLE h = static_cast<HANDLE>(handle_.exchange(INVALID_HANDLE_VALUE));
	if (h != INVALID_HANDLE_VALUE) {
		CancelIoEx(h, NULL);
		if (isServerPipe_) {
			DisconnectNamedPipe(h);
		}
		CloseHandle(h);
	}
	isServerPipe_ = false;
}

Listener::Listener() noexcept : pendingPipe_(INVALID_HANDLE_VALUE) {}

Listener::~Listener() {
	close();
	HANDLE h = static_cast<HANDLE>(pendingPipe_.exchange(INVALID_HANDLE_VALUE));
	if (h != INVALID_HANDLE_VALUE) {
		CloseHandle(h);
	}
}

static HANDLE createPipeInstance(const std::string& endpoint, bool first) {
	DWORD openMode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
	if (first) {
		openMode |= FILE_FLAG_FIRST_PIPE_INSTANCE;
	}
	return CreateNamedPipeA(
		endpoint.c_str(),
		openMode,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		256 * 1024,
		256 * 1024,
		0,
		NULL);
}

bool Listener::listen(const std::string& endpoint) {
	close();
	HANDLE stale = static_cast<HANDLE>(pendingPipe_.exchange(INVALID_HANDLE_VALUE));
	if (stale != INVALID_HANDLE_VALUE) {
		CloseHandle(stale);
	}
	closing_.store(false);
	endpoint_ = endpoint;
	HANDLE h = createPipeInstance(endpoint_, true);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	pendingPipe_.store(h);
	return true;
}

Stream Listener::accept() {
	ScopedWinEvent ev;
	if (ev.h == NULL) {
		return Stream();
	}

	while (!closing_.load()) {
		HANDLE h = static_cast<HANDLE>(pendingPipe_.load());
		if (h == INVALID_HANDLE_VALUE) {
			h = createPipeInstance(endpoint_, false);
			if (h == INVALID_HANDLE_VALUE) {
				return Stream();
			}
			pendingPipe_.store(h);
		}

		ResetEvent(ev.h);
		OVERLAPPED ov{};
		ov.hEvent = ev.h;

		BOOL ok = ConnectNamedPipe(h, &ov);
		DWORD err = ok ? ERROR_SUCCESS : GetLastError();

		bool connected = false;
		if (ok || err == ERROR_PIPE_CONNECTED) {
			connected = true;
		} else if (err == ERROR_IO_PENDING) {
			while (!closing_.load()) {
				DWORD wr = WaitForSingleObject(ev.h, 150);
				if (wr == WAIT_OBJECT_0) {
					DWORD dummy = 0;
					connected = GetOverlappedResult(h, &ov, &dummy, FALSE) != FALSE;
					break;
				}
				if (wr != WAIT_TIMEOUT) {
					break;
				}
			}
			if (!connected) {
				CancelIoEx(h, &ov);
				DWORD dummy = 0;
				GetOverlappedResult(h, &ov, &dummy, TRUE);
			}
		}

		if (closing_.load()) {
			HANDLE cur = static_cast<HANDLE>(pendingPipe_.exchange(INVALID_HANDLE_VALUE));
			if (cur != INVALID_HANDLE_VALUE) {
				if (connected) {
					DisconnectNamedPipe(cur);
				}
				CloseHandle(cur);
			}
			return Stream();
		}

		if (connected) {
			pendingPipe_.store(createPipeInstance(endpoint_, false));
			return Stream(h, true);
		}

		pendingPipe_.exchange(INVALID_HANDLE_VALUE);
		CloseHandle(h);
	}
	return Stream();
}

void Listener::close() noexcept {
	closing_.store(true);
	HANDLE h = static_cast<HANDLE>(pendingPipe_.load());
	if (h != INVALID_HANDLE_VALUE) {
		CancelIoEx(h, NULL);
	}
}

bool Listener::isListening() const noexcept {
	return !closing_.load() && pendingPipe_.load() != INVALID_HANDLE_VALUE;
}

#else // POSIX (macOS / Linux)

static void disableSigPipe(int fd) noexcept {
#ifdef SO_NOSIGPIPE
	int one = 1;
	::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#else
	(void)fd;
#endif
}

Stream::Stream() noexcept : fd_(-1) {}

Stream::Stream(int fd) noexcept : fd_(fd) {
	if (fd >= 0) {
		disableSigPipe(fd);
		::fcntl(fd, F_SETFD, FD_CLOEXEC);
	}
}

Stream::~Stream() {
	close();
}

Stream::Stream(Stream&& other) noexcept : fd_(other.fd_.exchange(-1)) {}

Stream& Stream::operator=(Stream&& other) noexcept {
	if (this != &other) {
		close();
		fd_.store(other.fd_.exchange(-1));
	}
	return *this;
}

Stream Stream::connect(const std::string& endpoint) {
	sockaddr_un addr{};
	addr.sun_family = AF_UNIX;
	if (endpoint.size() >= sizeof(addr.sun_path)) {
		return Stream();
	}
	std::strncpy(addr.sun_path, endpoint.c_str(), sizeof(addr.sun_path) - 1);

	int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		return Stream();
	}
	disableSigPipe(fd);
	::fcntl(fd, F_SETFD, FD_CLOEXEC);

	if (::connect(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0) {
		::close(fd);
		return Stream();
	}
	return Stream(fd);
}

bool Stream::valid() const noexcept {
	return fd_.load() >= 0;
}

bool Stream::writeAll(const void* data, size_t bytes) {
	const auto* ptr = static_cast<const unsigned char*>(data);
	size_t remaining = bytes;
	int flags = 0;
#ifdef MSG_NOSIGNAL
	flags |= MSG_NOSIGNAL;
#endif
	while (remaining > 0) {
		int fd = fd_.load();
		if (fd < 0) {
			return false;
		}
		ssize_t n = ::send(fd, ptr, remaining, flags);
		if (n < 0) {
			if (errno == EINTR) {
				continue;
			}
			return false;
		}
		if (n == 0) {
			return false;
		}
		ptr += static_cast<size_t>(n);
		remaining -= static_cast<size_t>(n);
	}
	return true;
}

bool Stream::readAll(void* data, size_t bytes) {
	auto* ptr = static_cast<unsigned char*>(data);
	size_t remaining = bytes;
	while (remaining > 0) {
		int fd = fd_.load();
		if (fd < 0) {
			return false;
		}
		ssize_t n = ::recv(fd, ptr, remaining, 0);
		if (n < 0) {
			if (errno == EINTR) {
				continue;
			}
			return false;
		}
		if (n == 0) {
			return false;
		}
		ptr += static_cast<size_t>(n);
		remaining -= static_cast<size_t>(n);
	}
	return true;
}

void Stream::close() noexcept {
	int fd = fd_.exchange(-1);
	if (fd >= 0) {
		::shutdown(fd, SHUT_RDWR);
		::close(fd);
	}
}

Listener::Listener() noexcept : listenFd_(-1) {}

Listener::~Listener() {
	close();
}

bool Listener::listen(const std::string& endpoint) {
	close();
	closing_.store(false);

	// If an existing server responds on this socket, refuse to steal it.
	if (checkProcRunning() == 1) {
		return false;
	}

	sockaddr_un addr{};
	addr.sun_family = AF_UNIX;
	if (endpoint.size() >= sizeof(addr.sun_path)) {
		return false;
	}
	std::strncpy(addr.sun_path, endpoint.c_str(), sizeof(addr.sun_path) - 1);

	::unlink(endpoint.c_str());

	int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		return false;
	}
	::fcntl(fd, F_SETFD, FD_CLOEXEC);

	mode_t oldMask = ::umask(0077);
	int bound = ::bind(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
	::umask(oldMask);
	if (bound != 0) {
		::close(fd);
		return false;
	}

	if (::listen(fd, 16) != 0) {
		::close(fd);
		::unlink(endpoint.c_str());
		return false;
	}

	endpoint_ = endpoint;
	listenFd_.store(fd);
	return true;
}

Stream Listener::accept() {
	while (!closing_.load()) {
		int lfd = listenFd_.load();
		if (lfd < 0) {
			return Stream();
		}
		pollfd pfd{};
		pfd.fd = lfd;
		pfd.events = POLLIN;
		int pr = ::poll(&pfd, 1, 150);
		if (closing_.load()) {
			return Stream();
		}
		if (pr < 0) {
			if (errno == EINTR) {
				continue;
			}
			return Stream();
		}
		if (pr == 0 || !(pfd.revents & POLLIN)) {
			continue;
		}
		int clientFd = ::accept(lfd, nullptr, nullptr);
		if (clientFd < 0) {
			if (errno == EINTR) {
				continue;
			}
			if (closing_.load()) {
				return Stream();
			}
			continue;
		}
		return Stream(clientFd);
	}
	return Stream();
}

void Listener::close() noexcept {
	closing_.store(true);
	int fd = listenFd_.exchange(-1);
	if (fd >= 0) {
		// Self-connect wakes poll/accept immediately on macOS & Linux.
		if (!endpoint_.empty()) {
			Stream wake = Stream::connect(endpoint_);
			wake.close();
			::unlink(endpoint_.c_str());
		}
		::shutdown(fd, SHUT_RDWR);
		::close(fd);
	}
	endpoint_.clear();
}

bool Listener::isListening() const noexcept {
	return !closing_.load() && listenFd_.load() >= 0;
}

#endif

} // namespace qwipc

bool getFolderLocation(char* p, int pSize) {
	if (p == nullptr || pSize < 2) {
		return false;
	}
	p[0] = '\0';

	char hPath[512] = {0};
	const char* ePath = std::getenv("HOME");
	if (ePath == nullptr || ePath[0] == '\0') {
		ePath = std::getenv("USERPROFILE");
		if (ePath == nullptr || ePath[0] == '\0') {
#ifdef WIN32
			HRESULT result = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, hPath);
			if (SUCCEEDED(result)) {
				ePath = hPath;
			} else {
				xmprintf(0, "ERROR: cannot find a path to the config file\n");
				return false;
			}
#else
			struct passwd* pw = ::getpwuid(::getuid());
			if (pw == nullptr || pw->pw_dir == nullptr) {
				xmprintf(0, "ERROR: cannot find a path to the config file\n");
				return false;
			}
			ePath = pw->pw_dir;
#endif
		}
	}

	std::filesystem::path dir = std::filesystem::path(ePath) / ".qwtw";
	std::string s = dir.string();
	std::snprintf(p, static_cast<size_t>(pSize), "%s", s.c_str());
	return true;
}

int checkProcRunning() {
	using namespace qwipc;
	std::string ep = endpointPath();
	Stream s = Stream::connect(ep);
	if (!s.valid()) {
		xmprintf(3, "checkProcRunning(): no listener at %s\n", ep.c_str());
		return 0;
	}
	IpcHello hello{};
	hello.magic = kHelloMagic;
	hello.version = kProtoVer;
	hello.role = RolePing;
	if (!s.writeAll(&hello, sizeof(hello))) {
		return 0;
	}
	uint32_t ack = 0;
	if (!s.readAll(&ack, sizeof(ack)) || ack != kPingAck) {
		return 0;
	}
	xmprintf(3, "checkProcRunning(): qwproc is alive at %s\n", ep.c_str());
	return 1;
}
