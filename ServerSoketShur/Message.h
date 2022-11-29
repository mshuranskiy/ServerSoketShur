#pragma once
#include"framework.h"

struct MessageHeader
{
	int to;
	int from;
	int type;
	int size;
};

enum MessageTypes
{
	MT_INIT,
	MT_EXIT,
	MT_GETDATA,
	MT_DATA,
	MT_NODATA,
	MT_CONFIRM,
	MT_DECLINE,
	MT_REFRESH,
	MT_HISTORY,
	MT_LAST_MESSAGES,
	MT_SUPPORT_SERVER_INIT
};

enum MessageRecipients
{
	MR_BROKER = 1,
	MR_ALL = 50,
	MR_USER = 100,
	MR_SUPPORT_SERVER = 300
};

class Message
{
public:
	MessageHeader header = { 0 };
	string data;
	static int ClientID;
	Message() {};
	~Message() {};
	Message(int to, int from, int type=MT_DATA, const string& data = "");

	void Send(CSocket& s);
	int Receive(CSocket& s);

	static void Send(CSocket& s, int to, int from, int action, const string& data = "");
	static Message Send(int to, int action = MT_DATA, const string& data = "");
	static Message SendForSupportServer(int to, int action = MT_DATA, const string& data = "");



};

