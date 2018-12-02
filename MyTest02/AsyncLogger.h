#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
#include <map>

using timepoint = std::chrono::system_clock::time_point;
using systemclock = std::chrono::system_clock;

class LogRecord;
class LogDetailMsg;
class AsyncLogger;
class Loggers;

enum LogRecordType {
	LOG_CMD = 0,
	LOG_MSG,
	LOG_EXIT,
    LOG_REQ,
    LOG_RSP
};

struct Loggers_Name {
    const std::string ROOT = std::string("ROOT");
};

class Loggers {

public:
    static void initLoggers();
    static void addLogger(std::string const& name, AsyncLogger* const logger);
    static AsyncLogger* getLogger(std::string const& name);
    static AsyncLogger* log(std::string const& msg);
    static std::atomic<int> g_loggerId;
private:
    static std::map<std::string, AsyncLogger*> loggers;
};

class AsyncLoggers {

public:
    static void initLoggers();
    static void addLogger(std::string const& name, AsyncLogger* const logger);
    static AsyncLogger* getLogger(std::string const& name);
    static AsyncLogger* log(std::string const& msg);
    static std::atomic<int> g_loggerId;
private:
    AsyncLoggers();
    static AsyncLoggers* instance;
    static std::map<std::string, AsyncLogger*> loggers;
    static AsyncLoggers* getInstance();
};

class AsyncLogger {

public:
	AsyncLogger(const std::string name);
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

	bool createThread();

	int m_loggerId;
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
};

class LogDetailMsg {
public:
    LogDetailMsg(const void* _data) : m_data(_data) {}
    ~LogDetailMsg() {
        delete m_data;
        m_data = 0;
    }

    const void* getData() const { return m_data; }
private:
    const void* m_data;
};

class LogRecord {
public:
	LogRecord(const std::string _msg) : m_type(LOG_MSG), m_msg(_msg), m_detailMsg(nullptr) {};
	LogRecord(LogRecordType _type, const std::string _msg) : m_type(_type), m_msg(_msg), m_detailMsg(nullptr) {};
	LogRecord(LogRecordType _type, LogDetailMsg* _dMsg) : m_type(_type), m_msg(""), m_detailMsg(_dMsg) {};
	~LogRecord() {
        delete m_detailMsg;
        m_detailMsg = 0;
    };
	std::string getMsg() const {	return m_msg; 	}
	LogRecordType getLogRecordType() { return m_type; }
	LogDetailMsg* getLogDetailMsg() const { return m_detailMsg; }

private:
	LogRecordType m_type;
	std::string m_msg;
    LogDetailMsg* m_detailMsg;
};

