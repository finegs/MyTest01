#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
#include <map>

#include <LogDetailMsg.h>
#include <LogRecord.h>

using timepoint = std::chrono::system_clock::time_point;
using systemclock = std::chrono::system_clock;

class LogRecord;
class LogDetailMsg;
class AsyncLogger;
class AsyncLoggers;

struct Loggers_Name {
    const std::string ROOT = std::string("ROOT");
};

class AsyncLoggers {

public:
    static void initLoggers();
    static void addLogger(const std::string& name, AsyncLogger* logger);
    static AsyncLogger* getLogger(const std::string& name);
    static AsyncLogger* log(const std::string& msg);
    static std::atomic<int> g_loggerId;
    static AsyncLoggers* getInstance();
private:
    AsyncLoggers() {};
    static AsyncLoggers* instance;
    static std::map<const std::string&, AsyncLogger*> loggers;
};

class AsyncLogger {

public:
	AsyncLogger(const std::string& name);
	~AsyncLogger();

	bool startLogProducer();
	bool shutdownLogProducer();
	void log(LogRecord* rec);
	void postMsg(LogDetailMsg* msg);
    void log(std::string const& msg);
	void run();
	void runLogProduce();
	void pause();
	void resume();
	void shutdown();

    const std::string getName() const { return m_name; };
	int getLoggerId() { return m_loggerId; }
    void setPauseDelayMils(int _pauseWaitMils) { m_pauseWaitMils = _pauseWaitMils;    }

	std::thread::id getThreadId() {
		if (m_thread) m_thread->get_id();
		return std::thread::id();
	}

private:
	const std::string m_name;
	const int m_loggerId;
	std::atomic<bool> m_exit;
	std::atomic<bool> m_run;
    std::atomic<bool> m_pause;
	std::atomic<bool> m_runLogProducer;
	std::queue<LogRecord*> m_queue;
	std::thread* m_thread;
	std::thread* m_thread_LogProducer;
	std::mutex m_mtx;
	std::condition_variable m_cv;
    std::atomic<int>  m_pauseWaitMils;

	bool createThread();
};

