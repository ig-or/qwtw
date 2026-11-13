#pragma once

#include "qwproc.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class QWorker;
class QApplication;

class QProcInterface {
public:
	QProcInterface(QWorker& worker_, QApplication& app_);
	~QProcInterface();

	/// Bind local IPC listener and start accepting clients. Returns true on success.
	bool start();

	/// Return true if another qwproc instance is already listening.
	static bool runningAlready();

	void onCB(const CBPickerInfo& cbi_);
	void onClip(double t1, double t2, int clipGroup);

private:
	QWorker& worker;
	QApplication& app;

	qwipc::Listener listener_;
	std::atomic<bool> started_{false};
	std::atomic<bool> stopping_{false};
	std::thread acceptThread_;

	std::mutex cmdExecMutex_;

	struct ClientSlot {
		std::shared_ptr<qwipc::Stream> stream;
		std::thread worker;
	};
	std::mutex clientThreadsMutex_;
	std::vector<ClientSlot> clientSlots_;

	std::mutex subscribersMutex_;
	std::vector<std::shared_ptr<qwipc::Stream>> subscribers_;

	void stop();
	void acceptLoop();
	void handleCommandClient(std::shared_ptr<qwipc::Stream> stream);
	void broadcastEvent(const qwipc::ServerEvent& ev);
	void executeCommand(const qwipc::CmdRequest& req, const std::vector<double>& payload, qwipc::CmdReply& rep, bool& requestQuit);
};
