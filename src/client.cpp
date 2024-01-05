#include <iostream>
#include <cstring>
#include "zmq.hpp"
#include <string>
#include <thread>
#include <string>
#include <unistd.h>

using namespace std;

void send_message(string message_string, zmq::socket_t &socket) {
    zmq::message_t message_back(message_string.size());
    memcpy(message_back.data(), message_string.c_str(), message_string.size());
    if (!socket.send(message_back, zmq::send_flags::none)) {
        cout << "Send message error." << "\n";
    }
}

string receive_message(zmq::socket_t &socket) {
    zmq::message_t message_main;
    socket.recv(&message_main);
    string answer(static_cast<char*>(message_main.data()), message_main.size());
    return answer;
}

void process_terminal(zmq::socket_t &pusher, string login) {
    string command = "";
    cout << "Enter command:" << "\n";
    while (cin >> command) 
    {
        if (command == "send") {
            cout << "Enter nickname of recipient:" << "\n";
            string recipient = "";
            cin >> recipient;
            cout << "Enter your message:" << "\n";
            string client_message = "";
            char a;
            cin >> a;
            getline(cin, client_message);
            string message_string = "send " + recipient + " " + login + " " + a + client_message;
            send_message(message_string, pusher);

        } else if (command == "history") {
            string message_string = "history";
            send_message(message_string, pusher);

        } else if (command == "exit") {
            send_message("exit " + login, pusher);
            break;

        } else {
            cout << "There is no such command." << "\n";
        }
        cout << "Enter command:" << "\n";
    }
}

void process_server(zmq::socket_t &puller) {
    while (1) 
    {
        string command = "";
        string recieved_message = receive_message(puller);
        for (char i: recieved_message) {
            if (i != ' ') {
                command += i;
            } else {
                break;
            }
        }

        if (command == "send") {
            int i;
            string recipient = "", sender = "", mes_to_me = "";
            for (i = 5; i < recieved_message.size(); ++i) {
                if (recieved_message[i] != ' ') {
                    recipient += recieved_message[i];
                } else {
                    break;
                }
            }
            ++i;
            for (i; i < recieved_message.size(); ++i) {
                if (recieved_message[i] != ' ') {
                    sender += recieved_message[i];
                } else {
                    break;
                }
            }
            ++i;
            for (i; i < recieved_message.size(); ++i) {
                mes_to_me += recieved_message[i];
            }
            cout << "Message from " << sender << ":" << "\n" << mes_to_me << "\n";

        } else if (command == "history") {
            string history;
            for (int i = 8; i < recieved_message.size(); ++i) {
                history += recieved_message[i];
            }
            cout << history << "\n";

        } else if (command == "no") {
            cout << "There is no such user." << "\n";

        } else if (command == "exit") {
            break;
        }
    }
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t socket_for_login(context, ZMQ_REQ);
    socket_for_login.connect("tcp://localhost:4042");
    
    cout << "Enter login: " << "\n";
    string login = "";
    cin >> login;
    send_message(to_string(getpid()) + " " + login, socket_for_login);
    string recieved_message = receive_message(socket_for_login);

    if (recieved_message == "0") {
        cout << "login is already used" << "\n";
        _exit(0);
    } else if (recieved_message == "1") {
        cout << "========= All commands ========== \n              send \n            history \n              exit \n"; 
        cout << "================================= \n";

        zmq::context_t context1(1);
        zmq::socket_t puller(context1, ZMQ_PULL); // прием сообщений от сервера
        puller.connect("tcp://localhost:3" + to_string(getpid()));

        zmq::context_t context2(1);
        zmq::socket_t pusher(context2, ZMQ_PUSH); // отправка сообщений клиенту
        pusher.connect("tcp://localhost:3" + to_string(getpid() + 1));

        thread thr;
        thr = thread(process_server, ref(puller));
        thr.detach();
        process_terminal(pusher, login);
        thr.join();

        context1.close();
        context2.close();
        puller.disconnect("tcp://localhost:3" + to_string(getpid()));
        pusher.disconnect("tcp://localhost:3" + to_string(getpid() + 1));
        puller.close();
        pusher.close();
    }
    socket_for_login.disconnect("tcp://localhost:4042");
    socket_for_login.close();
    context.close();
    return 0;
}
