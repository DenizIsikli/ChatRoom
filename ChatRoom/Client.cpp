#include "Client.h"
#include "GlobalConstants.h"
#include <iostream>
#include <thread>


Client::Client(const std::string& serverIP, unsigned short serverPort) : serverIP(""), serverPort(0), clientSocket(INVALID_SOCKET), exit_flag(false) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed.");
    }
}

Client::~Client() {
    if (clientSocket != INVALID_SOCKET) {
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);
    }
    WSACleanup();
}

void Client::connectToServer(const std::string &ip, unsigned short port) {
    serverIP = ip;
    serverPort = port;

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Socket creation failed.");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        WSACleanup();
        throw std::runtime_error("Could not connect to server.");
    }
}

void Client::run() {
    std::thread sendThread(&Client::send_message, this);
    std::thread recvThread(&Client::receive_message, this);

    sendThread.join();
    recvThread.join();
}

void Client::send_message() {
    std::string msg;
    std::cout << "Enter your name: ";
    std::getline(std::cin, msg);
    send(clientSocket, msg.c_str(), msg.length(), 0);

    while (!exit_flag) {
        std::getline(std::cin, msg);
        if (msg == "#exit") {
            exit_flag = true;
        }
        else {
            send(clientSocket, msg.c_str(), msg.length(), 0);
        }
    }
}

void Client::receive_message() {
    char buffer[1024];
    while (!exit_flag) {
        ZeroMemory(buffer, 1024);
        int bytesReceived = recv(clientSocket, buffer, 1024, 0);
        if (bytesReceived > 0) {
            std::string colorMsg = colors[rand() % colors.size()] + std::string(buffer, bytesReceived) + color_reset;
            std::cout << "\r" << colorMsg << std::endl;
            reprintPrompt();
        }
        else if (bytesReceived <= 0) {
            exit_flag = true;
        }
    }
}

void Client::reprintPrompt() const {
    std::cout << "\r" << std::string(100, ' ') << "\rYou: ";
    std::cout.flush();
}

const std::vector<std::string> Client::colors = {
    "\033[0;31m", // Red
    "\033[0;32m", // Green
    "\033[0;33m", // Yellow
    "\033[0;34m", // Blue
    "\033[0;35m", // Magenta
    "\033[0;36m"  // Cyan
};

const std::string Client::color_reset = "\033[0m";
