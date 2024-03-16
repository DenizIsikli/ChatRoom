#pragma once
#include "ClientStruct.h"

#include <vector>
#include <mutex>
#include <string>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include <random>
#include <string>
#include <mutex>


class Server {
public:
    Server();
    ~Server();

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> distrib;

    void run();
    void stop();
    static BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

    static const std::vector<std::string> colors;
    static const std::string color_reset;

private:
    SOCKET server_socket;
    std::mutex client_list_mtx;
    std::vector<ClientStruct> client_list;

    bool isExitFlagSet() const;
    void closeClientSockets();
    void handle_client(int client_num);
    void broadcast_message(const std::string& msg, int sender_id);
    void remove_client(int id);
    
};
