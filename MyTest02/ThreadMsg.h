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
	WorkerThreadMsg(WorkerThreadMsgType _id, const UserData* _msg) : 
        msgType(_id), 
        msgSeq(g_seq++), 
        msgDetail(_msg) 
    {
    }

	~WorkerThreadMsg() 
    {
		//if (!msg) return;
		//delete msg;
		//msg = nullptr;
	}

    WorkerThreadMsgType getMsgType() const { return msgType; }
    int getMsgSeq() const { return msgSeq; }
    const UserData* getMsgDetail() const { return msgDetail; }
private:
	WorkerThreadMsgType msgType;
	int msgSeq;
	const UserData* msgDetail;
};
