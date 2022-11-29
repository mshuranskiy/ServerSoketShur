#pragma once
#include "Message.h"

class Session
{
public:
	int id;
	string name;
	queue<Message> messages;
	CCriticalSection cs;
	chrono::steady_clock::time_point LastActive = chrono::steady_clock::now();
	Session(int id, string name);
	~Session() {};

	void MessageAdd(Message& m);
	void MessageSend(CSocket& s);
	void RefreshUsers(CSocket& s, string users);
	void SetLastActive();
};

