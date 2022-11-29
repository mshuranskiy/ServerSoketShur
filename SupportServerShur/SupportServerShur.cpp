// ClientSoketShur.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "SupportServerShur.h"
#include "../ServerSoketShur/Message.h"
#include "sqlite3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

map<int, string> Users;
CCriticalSection cs;
int from;


void checkinjection(string& myrequest)
{
    bool injection=false;
    for (int i = 0; i < myrequest.size(); i++)
    {
        if (myrequest[i] == ';')
        {
            myrequest.erase(i,1);
        }
        if (myrequest[i] == '\'')
        {
            myrequest.erase(i, 1);
            myrequest.insert(i, 1, '`');
        }
    }
}


void RefreshUsers(string str)
{
    Users.clear();
    stringstream NamesAndIds(str);
    while (true)
    {
        int user_id;
        string user_name;
        NamesAndIds >> user_id;
        if (int(user_id) == -1)
            break;
        NamesAndIds >> user_name;
        Users[int(user_id)] = user_name;
    }
}


static int callback(void* data, int argc, char** argv, char** azColName)
{
    string message = argv[2];
    Message::SendForSupportServer(from, MT_HISTORY, message);
    return 0;
}


void ProcessMessages()
{
    sqlite3* db;
    char* zErrMsg = 0;
    string sql;
    bool injection = false;
    auto rc = sqlite3_open("MessageDB.db", &db);
    while (true)
    {
        Message m = Message::SendForSupportServer(MR_BROKER, MT_GETDATA);
        switch (m.header.type)
        {
        case MT_DATA:
        {
            cs.Lock();
            cout << m.data << endl;
            cs.Unlock();
            checkinjection(m.data);
            sql = "INSERT INTO Messages (m_from, m_to, m_text) " \
                  "VALUES ('" + to_string(m.header.from) + "','" + to_string(m.header.to) + "','" + m.data + "'); ";
            rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &zErrMsg);
            break;
        }
        case MT_LAST_MESSAGES:
        {
            from = m.header.from;
            sql = "SELECT * FROM Messages WHERE Messages.m_to=50 or Messages.m_to='"+to_string(m.header.from)+"' or Messages.m_from='" + to_string(m.header.from) + "'";
            rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
            break;
        }
        default:
            Sleep(100);
            break;
        }
    }
    sqlite3_close(db);
    cout << "Переподключитесь" << endl;
}


void Client()
{
    AfxSocketInit();

    Message m = Message::SendForSupportServer(MR_BROKER, MT_SUPPORT_SERVER_INIT, "Support_Server");

    thread t(ProcessMessages);
    t.detach();
    while (true)
    {

    }
}



int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);
    setlocale(LC_ALL, "Russian");
    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: code your application's behavior here.
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: code your application's behavior here.
            Client();
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
