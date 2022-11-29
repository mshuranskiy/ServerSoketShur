#include "pch.h"
#include "Message.h"


string GetLastErrorString(DWORD ErrorID = 0)
{
	if (!ErrorID)
		ErrorID = GetLastError();
	if (!ErrorID)
		return string();

	LPSTR pBuff = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, ErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&pBuff, 0, NULL);
	string s(pBuff, size);
	LocalFree(pBuff);

	return s;
}

int Message::ClientID = 0;

Message::Message(int to, int from, int action, const string& data)
{
	this->data = data;
	this->header = { to, from, action, int(data.length()) };
}

void Message::Send(CSocket& s)
{
	s.Send(&header, sizeof(MessageHeader));
	if (header.size)
	{
		s.Send(data.c_str(), header.size);
	}
}

int Message::Receive(CSocket& s)
{
	if (!s.Receive(&header, sizeof(MessageHeader)))
		return MT_NODATA;
	if (header.size)
	{
		vector<char> v(header.size);
		s.Receive(&v[0], header.size);
		data = string(&v[0], header.size);
	}
	return header.type;
}

void Message::Send(CSocket& s, int to, int from, int action, const string& data)
{
	Message m(to, from, action, data);
	m.Send(s);
}

Message Message::Send(int to, int action, const string& data)
{
	CSocket s;
	s.Create();
	if (!s.Connect("127.0.0.1", 12345))
	{
		throw runtime_error(GetLastErrorString());
	}
	Message m(to, ClientID, action, data);
	m.Send(s);
	if (m.Receive(s) == MT_INIT)
	{
		ClientID = m.header.to;
	}
	return m;
}

Message Message::SendForSupportServer(int to, int action, const string& data)
{
	CSocket s;
	s.Create();
	if (!s.Connect("127.0.0.1", 12345))
	{
		throw runtime_error(GetLastErrorString());
	}
	Message m(to, ClientID, action, data);
	m.Send(s);
	if (m.Receive(s) == MT_SUPPORT_SERVER_INIT)
	{
		ClientID = m.header.to;
	}
	return m;
}
