#include <iostream>
#include <WorkerThread.h>
#include <ThreadMsg.h>
#include <mutex>
#include <queue>
#include <assert.h>

using namespace std;

WorkerThread::WorkerThread(const char* name) : THREAD_NAME(name) 
{
}
WorkerThread::~WorkerThread() {
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

	while (m_run) {
		ThreadMsg* msg = nullptr;
		{
			std::unique_lock<std::mutex> lk(m_mutex);
			while (m_queue.empty())
				m_cv.wait(lk);

			if (m_queue.empty())
				continue;
			msg = m_queue.front();
			m_queue.pop();
		}
		
		if (!msg) continue;

		switch (msg->id) {
		case MSG_POST_USER_DATA:
		{
			assert(msg->msg != NULL);

			const UserData* userData = static_cast<const UserData*>(msg->msg);
                       

			cout << "[" << msg->seq << "] "<< userData->msg.c_str() << " on " << userData->year << " by " << THREAD_NAME << endl;
			delete userData;
			delete msg;
			break;
		}

		case MSG_TIMER:
		{

			cout << "[" << msg->seq << "] Timer expired on " << THREAD_NAME << endl;
			delete msg;
			msg = nullptr;
			break;
		}

		case MSG_EXIT_THREAD:
		{
			m_timerExit = true;
			timerThread.join();

			delete msg;

			unique_lock<mutex> lk(m_mutex);
			while (!m_queue.empty()) {
				msg = m_queue.front();
				m_queue.pop();
				delete msg;
			}

			cout << "Exit thread on " << THREAD_NAME << endl;

			m_run = false;

			break;
		}

		default:
			assert(false);
		}
	}
}

void WorkerThread::postMsg(const UserData* data)
{
	assert(m_thread);

	ThreadMsg* threadMsg = new ThreadMsg(MSG_POST_USER_DATA, data);

	unique_lock<mutex> lk(m_mutex);
	m_queue.push(threadMsg);
	m_cv.notify_one();
}

void WorkerThread::exitThread() {
	if (!m_thread)
		return;

	ThreadMsg* threadMsg = new ThreadMsg(MSG_EXIT_THREAD, 0);
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

		ThreadMsg* threadMsg = new ThreadMsg(MSG_TIMER, 0);

		unique_lock<mutex> lk(m_mutex);
		m_queue.push(threadMsg);
		m_cv.notify_one();
	}
}


//WorkerThread& WorkerThread::operator=(const WorkerThread& workerThread) {
//	WorkerThread wt(workerThread);
//	return wt;
//}