#include <iostream>
#include <WorkerThread.h>
#include <ThreadMsg.h>
#include <mutex>
#include <queue>
#include <assert.h>
#include <sstream>
#include <AsyncLogger.h>

using namespace std;

WorkerThread::WorkerThread(const char* name) : threadName(name) 
{
}
WorkerThread::~WorkerThread() 
{
	//delete THREAD_NAME;
	//THREAD_NAME = nullptr;
}

std::thread::id WorkerThread::getThreadId() {
	if (m_thread) m_thread->get_id();
	return std::thread::id();
}

std::thread::id WorkerThread::getCurrentThreadId() {
	return this_thread::get_id();
}

bool WorkerThread::createThread() 
{
	if (!m_thread)
		m_thread = new thread(&WorkerThread::process, this);
	return true;
}


void WorkerThread::process() 
{
	m_timerExit = false;
	m_run = true;
	thread timerThread(&WorkerThread::timerThread, this);

    stringstream ss;
	while (m_run) {
		WorkerThreadMsg* msg = nullptr;
		{
			std::unique_lock<std::mutex> lk(m_mutex);
			while (m_queue.empty())
				m_cv.wait(lk);

			if (m_queue.empty())
				continue;
			msg = m_queue.front();
			m_queue.pop();
		}

        ss.clear();
		
		if (!msg) continue;

		switch (msg->getMsgType()) {
		case MSG_POST_USER_DATA:
		{
			assert(msg->getMsgDetail() != NULL);

			UserData* userData = msg->getMsgDetail();
            
            ss << "[" << msg->getMsgSeq() << "] "<< userData->msg.c_str() << " on " << userData->year << " by " << threadName << endl;

            LogRecord* rec = new LogRecord(ss.str());

            AsyncLoggers::log(LoggersNames.ROOT, rec);

			delete msg;
            msg = nullptr;

			break;
		}

		case MSG_TIMER:
		{
            ss << "[" << msg->getMsgSeq() << "] Timer expired on " << threadName << endl;

            LogRecord* rec = new LogRecord(ss.str());

            AsyncLoggers::log(LoggersNames.ROOT, rec);

			delete msg;
			msg = nullptr;
			break;
		}

		case MSG_EXIT_THREAD:
		{
			m_timerExit = true;
			timerThread.join();

			delete msg;
            msg = nullptr;

			unique_lock<mutex> lk(m_mutex);

			while (!m_queue.empty()) {
				msg = m_queue.front();

                ss.clear();

                UserData* ud = msg->getMsgDetail();

                ss << "[" << msg->getMsgSeq() << "] [" << ((ud) ? ud->msg.c_str() : "" ) << "] on " << ( ud  ? ud->year : 0) << " by " << threadName << endl;

                LogRecord* rec = new LogRecord(ss.str());

                AsyncLoggers::log(LoggersNames.ROOT, rec);

				m_queue.pop();
				delete msg;
                msg = nullptr;
			}

            ss.clear();

			ss << "Exit thread on " << threadName << endl;

            LogRecord* rec = new LogRecord(ss.str());

            AsyncLoggers::log(LoggersNames.ROOT, rec);

			m_run = false;

			break;
		}

		default:
			assert(false);
		}
	}
}

void WorkerThread::postMsg(UserData* data)
{
	assert(m_thread);

	WorkerThreadMsg* threadMsg = new WorkerThreadMsg(MSG_POST_USER_DATA, data);

	unique_lock<mutex> lk(m_mutex);
	m_queue.push(threadMsg);
	m_cv.notify_one();
}

void WorkerThread::exitThread() {
	if (!m_thread) return;

	WorkerThreadMsg* threadMsg = new WorkerThreadMsg(MSG_EXIT_THREAD, 0);
	{
		lock_guard<mutex> lk(m_mutex);
		m_queue.push(threadMsg);
		m_cv.notify_one();
	}

	m_thread->join();
	delete m_thread;
	m_thread = 0;
}


void WorkerThread::timerThread() {
	while (!m_timerExit) 
	{
		this_thread::sleep_for(250ms);

		WorkerThreadMsg* threadMsg = new WorkerThreadMsg(MSG_TIMER, 0);

		unique_lock<mutex> lk(m_mutex);
		m_queue.push(threadMsg);
		m_cv.notify_one();
	}
}

WorkerThread& WorkerThread::operator=(const WorkerThread& workerThread) {
    exitThread();
	return *this;
}