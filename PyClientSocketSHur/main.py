import threading
from dataclasses import dataclass
import socket, struct, time

import Message
from Message import *

username = ""
Users = dict()
mutex = threading.Lock()

def print_users():
    print("Пользователи :")
    for k, v in Users.items():
        if v!="Support_Server":
            print(k, v)


def refresh_users(str):
    Users.clear()
    buf = str.split(' ')
    for number in range(0, len(buf) - 1, 2):
        Users[int(buf[number])] = buf[number + 1]


def print_menu():
    print(
        "Меню:\n"
        "1: Отправить сообщение пользователю пользователь->сообщение\n"
        "2: Отправить сообщение ввсем\n"
        "3: Показать активных пользователей\n"
        "0: Отключиться"
    )


def registration():
    while True:
        global username
        username = input("Введите имя пользователя \n")
        m = Message.SendMessage(MR_BROKER, MT_INIT, username)
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
            print(m.Data)
            mutex.release()
        if m.Header.htype == MT_HISTORY:
            mutex.acquire()
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
            print_menu()


def main():
    registration()
    t = threading.Thread(target=process_messages, daemon=True)
    t.start()
    print_menu()
    Message.SendMessage(MR_SUPPORT_SERVER, MT_LAST_MESSAGES)
    client_action()


if __name__ == "__main__":
    main()