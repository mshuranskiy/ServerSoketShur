using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace SharpClientSoketShur
{
    class Program
    {
        static Mutex mutexObj = new();
        static SortedDictionary<int, string> Users = new SortedDictionary<int, string>();
        public static string username;

        static void PrintUsers()
        {
            Console.WriteLine("Пользователи :");
            foreach (var user in Users)
            {
                if(user.Value!="Support_Server")
                    Console.Write($"{user.Value} ({user.Key});");
            }
            Console.WriteLine();
        }

        static void RefreshUsers(string str)
        {
            Users.Clear();
            string[] buf = str.Split(' ');
            for (int i = 0; i < buf.Length - 1; i = i + 2)
            {
                Users.Add(int.Parse(buf[i]), buf[i + 1]);
            }
        }

        static void PrintMenu()
        {
            Console.WriteLine("Меню:");
            Console.WriteLine("1: Отправить сообщение пользователю пользователь->сообщение");
            Console.WriteLine("2: Отправить сообщение ввсем");
            Console.WriteLine("3: Показать активных пользователей");
            Console.WriteLine("0: Отключиться");
        }

        static void Registration()
        {
            while (true)
            {
                Console.WriteLine("Введите имя пользователя");
                username=Console.ReadLine();
                Message m = Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_INIT, username);
                if (m.header.type == MessageTypes.MT_DECLINE)
                {
                   Console.WriteLine("Имя пользователя занято");
                }
                else
                {
                    RefreshUsers(m.data);
                    PrintUsers();
                    Console.WriteLine($"Авторизация прошла успешно, {username}");
                    break;
                }
            }
        }

        public static void ProcessMessages()
        {
            while (true)
            {
                var m = Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_REFRESH, Users.Count.ToString());
                if (m.header.type != MessageTypes.MT_DECLINE)
                {
                    RefreshUsers(m.data);
                }
                m = Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_GETDATA);
                switch (m.header.type)
                {
                    case MessageTypes.MT_DATA:
                        mutexObj.WaitOne();
                        Console.WriteLine($"{ m.data}");
                        mutexObj.ReleaseMutex();
                        break;
                    case MessageTypes.MT_HISTORY:
                        mutexObj.WaitOne();
                        Console.WriteLine($"{m.data}");
                        mutexObj.ReleaseMutex();
                        break;
                    case MessageTypes.MT_EXIT:
                        Console.WriteLine("Вы отключены от сервера");
                        m = Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_EXIT);
                        break;
                    default:
                        Thread.Sleep(1000);
                        break;
                }
            }
            Console.WriteLine("Переподключитесь");
        }

        static void ClientAction()
        {
            bool ExitFlag = true;
            while (ExitFlag)
            {
                int action;
                action = Convert.ToInt32(Console.ReadLine());
                switch (action)
                {
                    case 0:
                        {
                            Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_EXIT);
                            ExitFlag = false;
                            break;
                        }
                    case 1:
                        {
                            string bodymessage;
                            int toID;
                            mutexObj.WaitOne();
                            Console.WriteLine("Для пользователя ");
                            toID=Convert.ToInt32(Console.ReadLine()); ;
                            bodymessage=Console.ReadLine();
                            mutexObj.ReleaseMutex();
                            string message = username + " : " + bodymessage;
                            if (Users.ContainsKey(toID))
                            {
                                Message.send((MessageRecipients)toID, MessageTypes.MT_DATA, message);
                            }
                            break;
                        }
                    case 2:
                        {
                            string bodymessage;
                            mutexObj.WaitOne();
                            Console.WriteLine("Для всех: ");
                            bodymessage=Console.ReadLine();
                            mutexObj.ReleaseMutex();
                            string message = username + " : " + bodymessage;
                            Message.send(MessageRecipients.MR_ALL, MessageTypes.MT_DATA, message);
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

        private static void Main(string[] args)
        {
            Registration();
            Thread t = new Thread(ProcessMessages);
            t.IsBackground = true;
            t.Start();
            PrintMenu();
            Message.send(MessageRecipients.MR_SUPPORT_SERVER, MessageTypes.MT_LAST_MESSAGES);
            ClientAction();
        }
    }
}
