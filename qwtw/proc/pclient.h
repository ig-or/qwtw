#pragma once

#include "qwproc.h"
#include "qwtypes.h"
#include "qwtw.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

struct SpectrogramInfo;

struct SHMTest {
	SHMTest();
	~SHMTest();

#ifdef USEMARBLE
	int testInit(const std::string& mdp, const std::string& mpp, int level = 0);
#else
	int testInit(int level = 0);
#endif
	int qwtfigure(int n, unsigned int flags);
	int qwtspectrogram(int n, unsigned int flags);
	int spectrogram_info(const SpectrogramInfo& info);
	void qwtClipGroup(int gr);

	void stopQt();
	void onClose();

	void qwttitle(const char* s);
	void qwtxlabel(const char* s);
	void qwtylabel(const char* s);
	void qwtclear();
#ifdef USEMARBLE
	int qwtmap(int n);
#endif
	int qwtservice(int x);
#ifdef USEMATHGL
	int qwtmgl(int n);
	void qwtmgl_line(int size, double* x, double* y, double* z, const char* name, const char* style);
	void qwtmgl_mesh(const MeshInfo& info);
#endif
	void qsetloglevel(int level);
	void qwtsetimpstatus(int impStatus);
	int qwtplot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize);
	int qwtplot2(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize, double* time);

	void qwtremove(int id);
	int qwtsave_png(int id, char* filename);
	int qwtsetpos(int key, QWndPos& pos);
	int qwtchange(int id, double* x, double* y, double* z, double* time, int size);
#ifdef USE_QT3D
	void qwtplot3d(double* x, double* y, double* z, int size, const char* name, const char* style,
		int lineWidth, int symSize, double* time);
#endif
#ifdef ENABLE_UDP_SYNC
	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size);
	void qwtDisableCoordBroadcast();
#endif

	void qwtshowmw();
	void setCB(OnPCallback cb);
	void setClipCallback(OnClipCallback cb);
	void setCBTest1(CBTest_1 cb);
	void setCBTest2(CBTest_2 cb);

private:
	using PayloadChunk = std::pair<const double*, size_t>;

	int sendCommand(CmdHeader::QWCmd cmd, const char* text = nullptr, int test = 0);
	int sendCommand(CmdHeader::QWCmd cmd, int v, unsigned int flags = 0);
	bool sendTransaction(
		qwipc::CmdRequest& req,
		const std::vector<PayloadChunk>& chunks,
		qwipc::CmdReply& rep);

	bool connectChannels();
	void cbThreadF();

#ifdef USEMARBLE
	int startProc(const std::string& mdp, const std::string& mpp, int level);
#else
	int startProc(int level);
#endif

	std::mutex ioMutex_;
	qwipc::Stream cmdStream_;
	qwipc::Stream cbStream_;

	std::atomic<bool> stopCbThread_{false};
	std::thread cbThread_;

	std::mutex cbHandlerMutex_;
	OnPCallback pCallback = nullptr;
	OnClipCallback pClipCallback = nullptr;
	CBTest_1 cbTest1 = nullptr;
	CBTest_2 cbTest2 = nullptr;

	int status;
};
