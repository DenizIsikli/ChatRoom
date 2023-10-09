#include "Common.h"
#include <iostream>
#include <string>
#include <thread>
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_LEN 200
#define NUM_COLORS 6



bool exit_flag = false;
thread t_send, t_recv;

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

int eraseText(int cnt) {
    for(int i=0; i<cnt; i++)
        cout<<"\b \b";
    return 0;
}

 void send_message() {
    while(!exit_flag) {
        char str[MAX_LEN];
        cin.getline(str, MAX_LEN);
        if(strcmp(str, "exit") == 0) {
            CtrlHandler();
        }
        send(client_socket, str, MAX_LEN, 0);
    }
}

void recv_message() {
    while(!exit_flag) {
        char str[MAX_LEN];
        int bytes_received = recv(client_socket, str, MAX_LEN, 0);
        if(bytes_received == SOCKET_ERROR) {
            handle_error("recv() failed");
        }
        if(bytes_received == 0) {
            cout << "Server disconnected." << endl;
            break;
        }
        cout << color(0) << str << default_color << endl;
    }
}

int run_client() {
    if ((client_socket=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        handle_error("client socket creation failed");
    }

    struct sockaddr_in client{};
    client.sin_family = AF_INET;
    client.sin_port = htons(54000);
    client.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (struct sockaddr *)&client, sizeof(client)) == -1) {
        handle_error("connection to server failed");
    }

    char name[MAX_LEN];
    cout << "Enter your name: ";
    cin.getline(name, MAX_LEN);
    send(client_socket, name, MAX_LEN, 0);

    cout << colors[NUM_COLORS-1] << "\n\t  ====== Welcome to the chat-room ======   " << endl << default_color;

    t_send = thread(send_message);
    t_recv = thread(recv_message);
    t_send.join();
    t_recv.join();
    return 0;
}
