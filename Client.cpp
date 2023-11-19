#include <iostream>
#include <string>
#include <thread>
#include <WinSock2.h>
#include <windows.h>
#include <vector>
#include <algorithm> // Include for find function
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_LEN 200
#define NUM_COLORS 6

vector<string> names = {};
bool exit_flag = false;
thread t_send, t_recv;
int client_socket;

void CtrlHandler() {
    char str[MAX_LEN] = "exit";
    send(client_socket, str, MAX_LEN, 0);
    exit_flag = true;
    t_send.detach();
    t_recv.detach();
    closesocket(client_socket);
    WSACleanup();
    exit(0);
}

void send_message() {
    while (!exit_flag) {
        char str[MAX_LEN];
        cin.getline(str, MAX_LEN);
        if (strcmp(str, "exit") == 0) {
            CtrlHandler();
        }
        send(client_socket, str, MAX_LEN, 0);
    }
}

void recv_message() {
    while (!exit_flag) {
        char str[MAX_LEN];
        int bytes_received = recv(client_socket, str, MAX_LEN, 0);
        if (bytes_received == SOCKET_ERROR) {
            cerr << "recv() failed" << endl;
        }
        if (bytes_received == 0) {
            cout << "Server disconnected." << endl;
            break;
        }
        cout << str << endl;
    }
}

int run_client() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup() failed" << endl;
        return -1;
    }

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "client socket creation failed" << endl;
        WSACleanup();
        return -1;
    }

    struct sockaddr_in client{};
    client.sin_family = AF_INET;
    client.sin_port = htons(54000);
    client.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (struct sockaddr*)&client, sizeof(client)) == -1) {
        cerr << "Connection to server failed" << endl;
        closesocket(client_socket);
        WSACleanup();
        return -1;
    }

    char name[MAX_LEN];
    cout << "Enter your name: ";
    cin.getline(name, MAX_LEN);
    while (find(names.begin(), names.end(), string(name)) != names.end()) {
        cout << "Name already exists. Enter another name: ";
        cin.getline(name, MAX_LEN);
    }
    names.push_back(string(name));

    send(client_socket, name, MAX_LEN, 0);

    cout << "\n\t  ====== Welcome to the chat-room ======   " << endl;

    t_send = thread(send_message);
    t_recv = thread(recv_message);
    t_send.join();
    t_recv.join();

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
