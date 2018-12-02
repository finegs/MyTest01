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

static struct Loggers_Names {
    const std::string ROOT = std::string("ROOT");
} LoggersNames;

class AsyncLoggers {

public: 
    AsyncLoggers(AsyncLoggers const&) = delete;
    AsyncLoggers& operator=(AsyncLoggers const&) = delete;

public:
    static void initLoggers();
    static AsyncLogger* newLogger(const std::string& name);
    void addLogger(std::string name, AsyncLogger* logger);
    AsyncLogger* getLogger(const std::string& name);
    static AsyncLogger* log(std::string msg);
    static AsyncLogger* log(const std::string& loggerName, std::string msg);

    static std::atomic<int> g_loggerId;
    static AsyncLoggers* getInstance();
private:
    AsyncLoggers() {};
    static std::mutex sMutex;
    static AsyncLoggers* instance;
    std::map<std::string, AsyncLogger*> loggers;
};

class AsyncLogger {
    friend AsyncLoggers;
private:
	AsyncLogger(int loggerId, const std::string& name);
public:
	~AsyncLogger();

	bool startLogProducer();
	bool shutdownLogProducer();
	void log(LogRecord* rec);
	void postMsg(LogDetailMsg* msg);
    void log(std::string msg);
	void run();
	void runLogProduce();
	void pause();
	void resume();
	void shutdown();

    std::string getName() const { return m_name; };
	int getLoggerId() { return m_loggerId; }
    void setPauseDelayMils(int _pauseWaitMils) { m_pauseWaitMils = _pauseWaitMils;    }

	std::thread::id getThreadId() {
		if (m_thread) m_thread->get_id();
		return std::thread::id();
	}

private:
	const int m_loggerId;
	const std::string m_name;
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

