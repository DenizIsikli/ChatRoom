#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <random>
#include <string>
#include <WinSock2.h>
#include <ws2tcpip.h>

class Client {
public:
    Client(const std::string& serverIP, unsigned short serverPort);
    ~Client();

    void connectToServer(const std::string& ip, unsigned short port);
    void run();

    static const std::vector<std::string> colors;
    static const std::string color_reset;

private:
    SOCKET clientSocket;
    std::string serverIP;
    unsigned short serverPort;
    bool exit_flag;

    void send_message();
    void receive_message();
    void reprintPrompt() const;

};
