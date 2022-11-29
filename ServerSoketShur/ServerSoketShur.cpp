// ServerSoketShur.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "ServerSoketShur.h"
#include "Message.h"
#include "Session.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;


int maxID = MR_USER;
map<int, shared_ptr<Session>> sessions;
CCriticalSection cs;
int SupportServerID = MR_SUPPORT_SERVER;

void LaunchSupServer()
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcess(NULL, (LPSTR)"SupportServerShur.exe", NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}


void LaunchCppClient()
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcess(NULL, (LPSTR)"ClientSoketShur.exe", NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

void LaunchSharpClient()
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcess(NULL, (LPSTR)"C:/Users/User/source/repos/TRIS/ServerSoketShur/SharpClientSoketShur/bin/Debug/net6.0/SharpClientSoketShur.exe", NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}


string GetUsers()
{
    string NamesAndIds = "";
    for (auto& session : sessions)
    {
        NamesAndIds = NamesAndIds + to_string(session.second->id) + " " + session.second->name + " ";
    }
    return NamesAndIds;
}


void AFKCheck()
{
    int TimeLimit = 10000;
    while (true)
    {
        if (sessions.size() > 0)
        {
            for (auto& session : sessions)
            {
                if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now()
                    - session.second->LastActive).count() > TimeLimit)
                {
                    Message m(session.second->id, MR_BROKER, MT_EXIT);
                    session.second->MessageAdd(m);
                    cout << session.second->id << " отсутствует" << endl;
                    cout << "Сессия " + to_string(session.first) + " удалена" << endl;
                    sessions.erase(session.first);
                    break;
                }
            }
        }
        Sleep(TimeLimit);
    }
}

void ClientProcessing(SOCKET hSock)
{
    CSocket s;
    s.Attach(hSock);
    Message m;
    int code = m.Receive(s);
    switch (code)
    {
    case MT_INIT:
    {
        bool Error = false;
        if (!Error)
        {
            auto session = make_shared<Session>(++maxID, m.data);
            sessions[session->id] = session;
            Message::Send(s, session->id, MR_BROKER, MT_INIT, (GetUsers() + "-1"));
            //Message initm = { MR_ALL,session->id ,MT_DATA,("New user conected (" + to_string(session->id))+")"};
            /*for (auto& [id, session] : sessions)
            {
                if (id != initm.header.from)
                    session->MessageAdd(initm);
            }*/
            cout << session->name << " подключен" << endl;
            session->SetLastActive();
        }
        break;
    }
    case MT_SUPPORT_SERVER_INIT:
    {
        bool Error = false;
        if (!Error)
        {
            auto session = make_shared<Session>(SupportServerID, m.data);
            sessions[session->id] = session;
            Message::Send(s, session->id, MR_BROKER, MT_SUPPORT_SERVER_INIT);
            cout << session->name << " подключен" << endl;
            session->SetLastActive();
        }
        break;
    }
    case MT_EXIT:
    {
        sessions.erase(m.header.from);
        Message::Send(s, m.header.from, MR_BROKER, MT_CONFIRM);
        Message initm = { MR_ALL,m.header.from ,MT_DATA,("User unconected (" + to_string(m.header.from)) + ")" };
        for (auto& [id, session] : sessions)
        {
            if (id != initm.header.from)
                session->MessageAdd(initm);
        }
        break;
    }
    case MT_GETDATA:
    {
        auto iSession = sessions.find(m.header.from);
        if (iSession != sessions.end())
        {
            iSession->second->MessageSend(s);
            iSession->second->SetLastActive();
        }
        break;
    }
    case MT_HISTORY:
    {
        
        auto iSessionFrom = sessions.find(m.header.from);
        if (iSessionFrom != sessions.end())
        {
            iSessionFrom->second->SetLastActive();
            auto iSessionTo = sessions.find(m.header.to);
            if (iSessionTo != sessions.end() && iSessionTo != sessions.find(SupportServerID))
            {
                iSessionTo->second->MessageAdd(m);
            }
            else if (m.header.to == MR_ALL)
            {
                for (auto& [id, session] : sessions)
                {
                    if (id != m.header.from && id != SupportServerID)
                        session->MessageAdd(m);
                }
            }
        }
        break;
    }
    case MT_REFRESH:
    {
        if (stoi(m.data) != int(sessions.size()))
        {
            auto iSession = sessions.find(m.header.from);
            if (iSession != sessions.end())
            {
                Message::Send(s, iSession->second->id, MR_BROKER, MT_REFRESH, (GetUsers() + "-1"));
                cout << GetUsers() + "-1" << endl;
                cout << iSession->second->id << " обновлен" << endl;
            }
        }
        else
            Message::Send(s, m.header.from, MR_BROKER, MT_DECLINE);
        break;
    }
    default:
    {
        auto iSessionFrom = sessions.find(m.header.from);
        if (iSessionFrom != sessions.end())
        {
            iSessionFrom->second->SetLastActive();
            auto iSessionTo = sessions.find(m.header.to);
            if (iSessionTo != sessions.end() && iSessionTo != sessions.find(SupportServerID))
            {
                iSessionTo->second->MessageAdd(m);
            }
            else if (m.header.to == MR_ALL)
            {
                for (auto& [id, session] : sessions)
                {
                    if (id != m.header.from && id != SupportServerID)
                        session->MessageAdd(m);
                }
            }
            sessions[MR_SUPPORT_SERVER]->MessageAdd(m);
            sessions[MR_SUPPORT_SERVER]->SetLastActive();
        }
        break;
    }
    }
}

void Server()
{
    AfxSocketInit();
    CSocket Server;
    Server.Create(12345);
    cout << "Сервер запущен\n";
    thread t1(AFKCheck);
    t1.detach();
    LaunchSupServer();
    LaunchCppClient();
    LaunchSharpClient();
    while (true)
    {
        if (!Server.Listen())
            break;
        CSocket s;
        Server.Accept(s);
        thread t(ClientProcessing, s.Detach());
        t.detach();
    }
    Server.Close();
    cout << "Сервер остановлен\n";
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
            Server();
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
