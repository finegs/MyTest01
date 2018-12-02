#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

#include <LogDetailMsg.h>
#include <ThreadMsg.h>
#include <UserData.h>

class WorkerThread
{
public:
	WorkerThread(const char* threadName);

	~WorkerThread();

	bool createThread();

	void exitThread();

	std::thread::id getThreadId();
	
	static std::thread::id getCurrentThreadId();

	void postMsg(UserData* data);

private:
	WorkerThread(const WorkerThread&);
	WorkerThread& operator=(const WorkerThread&);

	void process();

	void timerThread();

	std::thread* m_thread;
	std::queue<WorkerThreadMsg*> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::atomic<bool>	m_timerExit;
	std::atomic<bool>   m_run;
	const char* threadName;
};