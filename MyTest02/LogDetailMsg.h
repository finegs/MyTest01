#pragma once

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
