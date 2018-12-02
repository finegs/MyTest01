#pragma once

#include <atomic>
#include <UserData.h>

enum WorkerThreadMsgType {
	MSG_POST_USER_DATA = 0,
	MSG_TIMER,
	MSG_EXIT_THREAD
};

class WorkerThreadMsg {
public:
    static std::atomic<int> g_seq;
	WorkerThreadMsg(WorkerThreadMsgType _MsgType, UserData* _msg) : 
        msgType(_MsgType), 
        msgSeq(g_seq++), 
        msgDetail(_msg) 
    {
    }

	~WorkerThreadMsg() 
    {
		if (!msgDetail) return;
		delete msgDetail;
        msgDetail = nullptr;
	}

    WorkerThreadMsgType getMsgType() const { return msgType; }
    int getMsgSeq() const { return msgSeq; }
    UserData* getMsgDetail() const { return msgDetail; }
private:
	WorkerThreadMsgType msgType;
	int msgSeq;
	UserData* msgDetail;
};
