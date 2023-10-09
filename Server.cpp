#include "Common.h"
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

vector<terminal> clients;
int seed = 0;
mutex cout_mtx, clients_mtx;

void handle_error(const char* message) {
    cerr << message << endl;
    cerr << "Error code: " << WSAGetLastError() << endl;
    WSACleanup();
    exit(0);
}



void set_name(int id, char name[]) {
    for (auto &client_ : clients) {
        if (client_.id == id) {
            client_.name = name;
        }
    }
}

void shared_print(string str) {
    lock_guard<mutex> guard(cout_mtx);
    cout << str;
}

void broadcast_message(string message, int sender_id) {
    char temp[MAX_LEN];
    strcpy_s(temp, message.c_str());

    for (auto &client_ : clients) {
        if (client_.id != sender_id) {
            int sentBytes = send(client_.socket, temp, static_cast<int>(strlen(temp)), 0);
            if (sentBytes == SOCKET_ERROR) {
                handle_error("send() failed");
            }
        }
    }
}

void end_connection(int id) {
    for (int i = 0; i < clients.size(); i++) {
        if (clients[i].id == id) {
            closesocket(clients[i].socket);
            clients.erase(clients.begin() + i);
        }
    }
}

void handle_client(SOCKET client_socket_, int id) {
    char name[MAX_LEN];
    recv(client_socket_, name, MAX_LEN, 0);
    set_name(id, name);
    shared_print(color(id) + name + " has joined the chat." + default_color);

    while (true) {
        char message[MAX_LEN];
        int bytes_received = recv(client_socket_, message, MAX_LEN, 0);
        if (bytes_received <= 0) {
            return;
        }
        if (strcmp(message, "exit") == 0) {
            shared_print(color(id) + name + " has left the chat." + default_color);
            end_connection(id);
            return;
        }

        broadcast_message(name + string(" : ") + message, id);
    }
}

int run_server() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        handle_error("WSAStartup() failed");
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        handle_error("socket() failed");
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(54000);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        handle_error("bind() failed");
    }

    if (listen(server_socket, 8) == SOCKET_ERROR) {
        handle_error("listen() failed");
    }

    cout << colors[NUM_COLORS - 1] << "\n\t  ====== Welcome to the chat-room ======   " << endl << default_color;
    cout << colors[NUM_COLORS - 1] << "\t  ======      Type 'exit' to quit     ======   " << endl << default_color;

    sockaddr_in client{};
    int client_len = sizeof(client);

    SOCKET client_socket_ = accept(server_socket, (sockaddr*)&client, &client_len);
    if (client_socket_ == INVALID_SOCKET) {
        handle_error("accept() failed");
    }

    seed++;
    thread thread(handle_client, client_socket_, seed);
    lock_guard<mutex> guard(clients_mtx);
    clients.push_back({seed, string("Anonymous"), client_socket_, (std::move(thread)) });

    // range based for loop for clients-size()
    for (auto &client_ : clients) {
        if (client_.th.joinable()) {
            client_.th.join();
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

