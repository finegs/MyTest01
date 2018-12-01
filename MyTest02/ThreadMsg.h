#pragma once

enum MSG {
	MSG_POST_USER_DATA = 0,
	MSG_TIMER,
	MSG_EXIT_THREAD
};

class ThreadMsg {
	static int g_seq;
public:
	ThreadMsg(MSG _id, const UserData* _msg) : id(_id), seq(g_seq++), msg(std::move(_msg)) {}
	~ThreadMsg() {
		//if (!msg) return;
		//delete msg;
		//msg = nullptr;
	}
public:
	MSG id;
	int seq;
	const UserData* msg;
};
