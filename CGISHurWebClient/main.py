import cgi
import threading
from dataclasses import dataclass
import socket, struct, time
from http.server import HTTPServer, BaseHTTPRequestHandler
import Message
from Message import *

username = "Dima"
Users = dict()
mutex = threading.Lock()
messages = []
clientId = 0
clientmsg = ''


def print_users():
    print("Пользователи :")
    for k, v in Users.items():
        print(k, v)


def refresh_users(str):
    Users.clear()
    buf = str.split(' ')
    for number in range(0, len(buf) - 1, 2):
        Users[int(buf[number])] = buf[number + 1]


class requestHandler(BaseHTTPRequestHandler):
    global messages, clientmsg
    def do_GET(self):
        self.send_response(200)
        self.send_header('content-type', 'text/html')
        self.end_headers()

        output = '<html><body><form method="POST" enctype="multipart/form-data">' \
                 '<h1>Welcome ' + username + '</h1>' \
                 '<h1>' + str(clientmsg) + '</h1>' \
                 '<input name="message" type="text" placeholder="New message">' \
                 '<input name="id" type="text" placeholder="id">' \
                 '<input name="SendToAll" type="checkbox" value = "1"/>' \
                 '<label for ="SendToAll" > Send To All </label><br/>' \
                 '<input type="submit" value="Send">' \
                 '<h1>Message List</h1><ul>'
        print(messages)
        for i in messages:
            output += '<li>' + i + '</li>'
        output += '</ul>'
        self.wfile.write(output.encode())

    def do_POST(self):
        ctype, pdict = cgi.parse_header(self.headers['content-type'])
        pdict['boundary'] = bytes(pdict['boundary'], "utf-8")
        content_len = int(self.headers.get('Content-length'))
        pdict['Content-Length'] = content_len
        if ctype == 'multipart/form-data':
            fields = cgi.parse_multipart(self.rfile, pdict)
            print(fields)
            id = fields.get('id')[0]
            if len(id) != 0 or fields.get('SendToAll')!=None:
                if(len(id)!=0):
                    id = int(id)
                text = username + " : "+fields.get('message')[0]
                try:
                 fields.get('SendToAll')[0]
                except:
                    Message.SendMessage(id, MT_DATA, text)
                    messages.append(text)
                else:
                    Message.SendMessage(MR_ALL, MT_DATA, text)
                    messages.append(text)

        self.send_response(301)
        self.send_header('content-type', 'text/html')
        self.send_header('Location', '/')
        self.end_headers()

def ProcessServer():
    server_address = ("", 8000)
    print("Web interface started")
    HTTPServer(server_address, requestHandler).serve_forever()

def registration():
    while True:
        global username
        #username = input("Введите имя пользователя \n")
        m = Message.SendMessage(MR_BROKER, MT_INIT, "Dima")
        refresh_users(m.Data)
        print_users()
        print(f'Авторизация прошла успешно {username}')
        break


def process_messages():
    exit_flag = True
    while exit_flag:
        m = Message.SendMessage(MR_BROKER, MT_REFRESH, str(len(Users)))
        if m.Header.htype != MT_DECLINE:
            refresh_users(str(m.Data))
        m = Message.SendMessage(MR_BROKER, MT_GETDATA)
        if m.Header.htype == MT_DATA:
            mutex.acquire()
            messages.append(m.Data)
            print(m.Data)
            mutex.release()
        if m.Header.htype == MT_HISTORY:
            mutex.acquire()
            messages.append(m.Data)
            print(m.Data)
            mutex.release()
        elif m.Header.htype == MT_EXIT:
            print("Вы отключены от сервера")
            m = Message.SendMessage(MR_BROKER, MT_EXIT)
            exit_flag = False
        else:
            time.sleep(1)


def client_action():
    exit_flag = True
    while exit_flag:
        action = (input())
        if action == "0":
            Message.SendMessage(MR_BROKER, MT_EXIT)
            exit_flag = False
        elif action == "1":
            mutex.acquire()
            to_id = int(input("Для пользователя "))
            body_message = input()
            mutex.release()
            message = username + ": " + body_message
            if to_id in Users:
                Message.SendMessage(to_id, MT_DATA, message)
        elif action == "2":
            mutex.acquire()
            body_message = input("Для всех: ")
            mutex.release()
            message = username + ": " + body_message
            Message.SendMessage(MR_ALL, MT_DATA, message)
        elif action == "3":
            print_users()
        else:
            print("Неверный ввод попробуйте ещё раз")


def main():
    registration()
    t = threading.Thread(target=process_messages, daemon=True)
    t.start()
    w = threading.Thread(target=ProcessServer)
    w.start()
    Message.SendMessage(MR_SUPPORT_SERVER, MT_LAST_MESSAGES)
    client_action()


if __name__ == "__main__":
    main()