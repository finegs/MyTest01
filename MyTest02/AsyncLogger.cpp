#include <AsyncLogger.h>
#include <cassert>

using namespace std;

AsyncLogger::AsyncLogger(int _loggerId, const std::string& _name) : m_loggerId(_loggerId), m_name(_name), m_pauseWaitMils(250) 
{
}

AsyncLogger::~AsyncLogger() 
{
}

bool AsyncLogger::createThread() {
	if(!m_thread)
		m_thread = new std::thread(&AsyncLogger::run, this);
	return true;
}

bool AsyncLogger::startLogProducer() {
    m_runLogProducer = true;
	if(!m_thread_LogProducer)
		m_thread_LogProducer = new std::thread(&AsyncLogger::runLogProduce, this);
	return true;
}

bool AsyncLogger::shutdownLogProducer() {
    if (!m_runLogProducer) return true;
    m_runLogProducer = false;
    if(m_thread_LogProducer) m_thread_LogProducer->join();
    return true;
}

void AsyncLogger::run() {
	m_exit = false;
	m_run = true;
    m_pause = false;

	startLogProducer();

	while (m_run) {

        if (m_pause) {
            this_thread::sleep_for(std::chrono::milliseconds(m_pauseWaitMils));
            continue;
        }

		LogRecord* rec = nullptr;
		{
			std::unique_lock<std::mutex> lk(m_mtx);
			while (m_queue.empty()) {
				m_cv.wait(lk);
			}
			
			if (m_queue.empty()) continue;
			rec = m_queue.front();
			m_queue.pop();
		}

		if (!rec) continue;

        switch (rec->getLogRecordType()) {
        case LOG_EXIT:
        {

            shutdownLogProducer();

            delete rec;

            std::unique_lock<std::mutex> lk(m_mtx);
            while (!m_queue.empty()) {
                rec = m_queue.front();

                std::cout << "[Exiting] " << rec->getMsg() << std::endl;

                m_queue.pop();
                delete rec;
            }

            m_run = false;
            break;
        }

        case LOG_RSP:
        {
            const LogDetailMsg* dMsg = rec->getLogDetailMsg();

            const timepoint* tp = static_cast<const timepoint*>(dMsg->getData());
            
            std::cout << "[" << tp << "] " << rec->getMsg() << std::endl;

            delete tp;
            delete rec;
            rec = 0;

            break;
        }

        case LOG_MSG:
        {
            std::cout << "[Msg] " << rec->getMsg() << std::endl;
            delete rec;
            rec = nullptr;
            break;
        }

        case LOG_CMD:
        {
            delete rec;
            rec = nullptr;
            break;
        }

        default:
            assert(false);
		}
	}
}

void AsyncLogger::log(LogRecord* rec) 
{
    assert(m_thread);

    std::unique_lock<std::mutex> lk(m_mtx);
    m_queue.push(rec);
    m_cv.notify_one();
}

void AsyncLogger::postMsg(LogDetailMsg* msg) 
{
    log(new LogRecord(LOG_RSP, msg));
}

void AsyncLogger::log(std::string msg) {
    log(new LogRecord(LOG_MSG, msg));
}

void AsyncLogger::shutdown() {
    if (!m_thread) return;

    LogRecord* rec = new LogRecord(LOG_EXIT, nullptr);
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_queue.push(rec);
        m_cv.notify_one();
    }

    m_thread->join();
    delete m_thread;
    m_thread = 0;
}

void AsyncLogger::pause() {
    m_pause = true;
}

void AsyncLogger::resume() {
    m_pause = false;
}

void AsyncLogger::runLogProduce() {
    while (m_runLogProducer) {

        std::this_thread::sleep_for(250ms);

        timepoint* tp = new timepoint(systemclock::now());

        LogDetailMsg* dMsg = new LogDetailMsg(tp);

        LogRecord* rec = new LogRecord(LOG_RSP, dMsg);
    }
}

AsyncLogger* AsyncLoggers::newLogger(const std::string& name) {
    AsyncLogger* logger;
    if (logger = AsyncLoggers::getInstance()->getLogger(name)) return logger;
    logger = new AsyncLogger(AsyncLoggers::g_loggerId++, name);

    AsyncLoggers::getInstance()->addLogger(logger->getName(), logger);
    logger->createThread();
    return logger;
}

void AsyncLoggers::initLoggers() {

    AsyncLogger* rootLogger = AsyncLoggers::newLogger(LoggersNames.ROOT);

    AsyncLoggers::getInstance()->addLogger(rootLogger->getName(), rootLogger);
}

void AsyncLoggers::addLogger(std::string name, AsyncLogger* logger) {
    if (loggers[name]) return;
    loggers[name] = logger;
}

AsyncLogger* AsyncLoggers::getLogger(const std::string& name)  {
    if (name.empty()) return loggers[LoggersNames.ROOT];
    return loggers[name];
}

AsyncLogger* AsyncLoggers::log(std::string msg) {
    AsyncLogger* logger = AsyncLoggers::getInstance()->getLogger(LoggersNames.ROOT);
    logger->log(std::move(std::string(msg)));
    return logger;
}

AsyncLogger* AsyncLoggers::log(const std::string& loggerName, std::string msg) {
    AsyncLogger* logger = AsyncLoggers::getInstance()->getLogger(loggerName);
    logger->log(std::move(std::string(msg)));
    return logger;
}

AsyncLoggers* AsyncLoggers::instance = nullptr;

AsyncLoggers* AsyncLoggers::getInstance() {

    if (!instance) {
        std::lock_guard<std::mutex> mLock(sMutex);

        instance = new AsyncLoggers();

        AsyncLoggers::initLoggers();
    }
    return instance;
}

