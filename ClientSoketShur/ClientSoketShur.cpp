// ClientSoketShur.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "ClientSoketShur.h"
#include "../ServerSoketShur/Message.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

map<int, string> Users;
CCriticalSection cs;
string username = "";

void PrintUsers()
{
    cout << "������������ :" << endl;
    for (auto& user : Users)
    {
        if(user.second!="Support_Server")
            cout << user.second << "(" << user.first << "); ";
    }
    cout << endl;
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

void printMenu()
{
    cout << "����:" << endl;
    cout << "1: ��������� ��������� ������������ ������������->���������" << endl;
    cout << "2: ��������� ��������� �����" << endl;
    cout << "3: �������� �������� �������������" << endl;
    cout << "0: �����������" << endl;
}

void Registration()
{
    while (true)
    {
        cout << "������� ��� ������������" << endl;
        cin >> username;
        cin.ignore();
        Message m = Message::Send(MR_BROKER, MT_INIT, username);
        if (m.header.type == MT_DECLINE)
        {
            cout << "��� ������������ ������" << endl;
        }
        else
        {
            RefreshUsers(m.data);
            PrintUsers();
            cout << "����������� ������ �������, " << username << endl;
            break;
        }
    }
}


void ProcessMessages()
{
    
    bool ExitFlag = true;
    while (ExitFlag)
    {
        Message m = Message::Send(MR_BROKER, MT_REFRESH, to_string(Users.size()));
        if (m.header.type != MT_DECLINE)
            RefreshUsers(m.data);
        m = Message::Send(MR_BROKER, MT_GETDATA);
        switch (m.header.type)
        {
        case MT_DATA:
        {
            cs.Lock();
            cout << m.data << endl;
            cs.Unlock();
            break;
        }
        case MT_HISTORY:
        {
            cs.Lock();
            cout << m.data << endl;
            cs.Unlock();
            break;
        }
        case MT_EXIT:
            cout << "�� ��������� �� �������" << endl;
            m = Message::Send(MR_BROKER, MT_EXIT);
            ExitFlag = false;
        default:
            Sleep(100);
            break;
        }
    }
    cout << "����������������" << endl;
}

void ClientAction()
{
    bool ExitFlag = true;
    while (ExitFlag)
    {
        int action;
        action = _getch();
        switch (action-48)
        {
        case 0:
        {
            Message::Send(MR_BROKER, MT_EXIT);
            ExitFlag = false;
            break;
        }
        case 1:
        {
            string bodymessage;
            int toID;
            cs.Lock();
            cout << "��� ������������ ";
            cin >> toID;
            cin.ignore();
            getline(cin, bodymessage);
            cs.Unlock();
            string message = username + " : " + bodymessage;
            auto user = Users.find(toID);
            if (user != Users.end())
            {
                Message::Send(user->first, MT_DATA, message);
            }
            break;
        }
        case 2:
        {
            string bodymessage;
            cs.Lock();
            cout << "��� ����: "<<endl;
            cin.clear();
            getline(cin, bodymessage);
            cs.Unlock();
            string message = username + " : " + bodymessage;
            Message::Send(MR_ALL, MT_DATA, message);
            break;
        }
        case 3:
        {
            PrintUsers();
            break;
        }
        default:
        {
            break;
        }
        }

    }
}

void Client()
{
    AfxSocketInit();

    Registration();

    thread t(ProcessMessages);
    t.detach();

    printMenu();

    Message::Send(MR_SUPPORT_SERVER, MT_LAST_MESSAGES);

    ClientAction();
    
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
