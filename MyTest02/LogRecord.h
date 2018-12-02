#pragma once

#include <string>
#include <LogDetailMsg.h>

enum LogRecordType {
    LOG_CMD = 0,
    LOG_MSG,
    LOG_EXIT,
    LOG_REQ,
    LOG_RSP
};


class LogRecord {
public:
    LogRecord(const std::string&& _msg) {
        m_msg.assign(_msg);
        m_type = LOG_MSG;
        m_detailMsg = nullptr;
    };
    LogRecord(LogRecordType _type, const std::string&& _msg) {
        m_msg.assign(_msg);
        m_type = _type;
        m_detailMsg = nullptr;
    };
    LogRecord(LogRecordType _type, const std::string&& _msg, LogDetailMsg* _dMsg) {
        m_msg.assign(_msg);
        m_type = _type;
        m_detailMsg = _dMsg;
    };
    ~LogRecord() {
        m_msg.clear();
        if (!m_detailMsg) return;
        delete m_detailMsg;
        m_detailMsg = 0;
    };
    std::string getMsg() const { return m_msg; }
    LogRecordType getLogRecordType() const { return m_type; }
    LogDetailMsg* getLogDetailMsg() const { return m_detailMsg; }

private:
    LogRecordType m_type;
    std::string m_msg;
    LogDetailMsg* m_detailMsg;
};

