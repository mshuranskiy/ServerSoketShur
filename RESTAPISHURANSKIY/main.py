from msg import *
import requests
clientId = 0
clientmsg = ''
UserName=""
mutex = threading.Lock()

def print_menu():
    print(
        "Меню:\n"
        "1: Отправить сообщение пользователю пользователь->сообщение\n"
        "2: Отправить сообщение ввсем\n"
        "0: Отключиться"
    )


def ProcessMessages():
    global clientId
    while True:
        a = SendRequest({'to':MR_BROKER, 'from':clientId, 'type': MT_GETDATA, 'data':''})
        if int(a['type']) == MT_DATA:
            print(a['data'])
        elif int(a['type']) == MT_HISTORY:
            print(a['data'])
        else:
            time.sleep(0.1)


def SendRequest(params):
    URL = "http://localhost:8989"
    r = requests.get(url=URL, json=params)
    return r.json()

def GetHistory():
    global clientId
    a = SendRequest({'to': MR_SUPSERVER, 'from': clientId, 'type': MT_LAST_MESSAGES, 'data': ""})

def SendInit():
    global clientId
    id = SendRequest({'to':MR_BROKER, 'from':'', 'type': MT_INIT, 'data': UserName})
    clientId = int(id['to'])

def Send(id, message):
    global clientId
    a = SendRequest({'to':id, 'from':clientId, 'type': MT_DATA, 'data':message})

def SendAll(message):
    global clientId
    a = SendRequest({'to': MR_ALL, 'from': clientId, 'type': MT_DATA, 'data': message})

def SendExit():
    global clientId
    a = SendRequest({'to': MR_BROKER, 'from': clientId, 'type': MT_EXIT, 'data': ''})

def Client():
        global clientId
        SendInit()
        GetHistory()
        w = threading.Thread(target=ProcessMessages)
        w.start()
        while True:
            action = (input())
            if action == "2":
                mutex.acquire()
                body_message = input("Для всех: ")
                mutex.release()
                message = UserName + ": " + body_message
                SendAll(message)
            elif action == "1":
                mutex.acquire()
                to_id = int(input("Для пользователя "))
                body_message = input()
                mutex.release()
                message = UserName + ": " + body_message
                Send(to_id, message)
            elif action == "0":
                SendExit()
                quit()
                break



if __name__ == '__main__':
    UserName = input("Введите имя пользователя \n")
    print_menu()
    Client()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
