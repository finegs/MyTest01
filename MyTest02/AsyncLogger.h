#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>

using timepoint = std::chrono::system_clock::time_point;
using systemclock = std::chrono::system_clock;

class LogRecord;
class LogDetailMsg;

enum LogRecordType {
	LOG_CMD = 0,
	LOG_MSG,
	LOG_EXIT,
    LOG_REQ,
    LOG_RSP
};


class AsyncLogger {

public:
	static int g_seq;

public:
	AsyncLogger(const std::string name);
	~AsyncLogger();

	bool startLogProducer();
	bool shutdownLogProducer();
	void log(LogRecord* rec);
	void postMsg(LogDetailMsg* msg);
	void run();
	void runLogProduce();
	void pause();
	void resume();
	void shutdown();

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

class LogRecord {
public:
	LogRecord(const std::string _msg) : m_type(LOG_MSG), m_msg(_msg) {};
	LogRecord(LogRecordType _type, const std::string _msg) : m_type(_type), m_msg(_msg) {};
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


class LogDetailMsg {
public:
    LogDetailMsg(void* _data) : m_data(_data) {}
    ~LogDetailMsg() {
        delete m_data;
        m_data = 0;
    }

    void* getData() const { return m_data; }
private:
    void* m_data;
};