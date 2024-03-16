#include "Server.h"
#include "Client.h"
#include "GlobalConstants.h"
#include <random>


Server::Server() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed.");
    }

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Socket creation failed.");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(server_socket);
        WSACleanup();
        throw std::runtime_error("Server socket binding failed.");
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(server_socket);
        WSACleanup();
        throw std::runtime_error("Server socket listen failed.");
    }
}

Server::~Server() {
    stop();
    WSACleanup();
}

void Server::run() {
    while (!exit_flag) {
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(server_socket, reinterpret_cast<SOCKADDR*>(&clientAddr), &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        ClientStruct newClient{};
        newClient.socket = clientSocket;

        {
            std::lock_guard<std::mutex> lock(client_list_mtx);
            client_list.push_back(std::move(newClient));
        }

        int clientIndex = static_cast<int>(client_list.size()) - 1;
        std::thread clientThread(&Server::handle_client, this, clientIndex);
        clientThread.detach();
    }
}

void Server::stop() {
    exit_flag = true;

    closeClientSockets();

    if (server_socket != INVALID_SOCKET) {
        shutdown(server_socket, SD_BOTH);
        closesocket(server_socket);
        server_socket = INVALID_SOCKET;
    }
}

bool Server::isExitFlagSet() const {
    return exit_flag;
}

void Server::closeClientSockets() {
    std::lock_guard<std::mutex> lock(client_list_mtx);
    for (auto& client : client_list) {
        shutdown(client.socket, SD_BOTH);
        closesocket(client.socket);
    }
    client_list.clear();
}

void Server::handle_client(int client_num) {
    SOCKET& client_socket = client_list[client_num].socket;

    client_list[client_num].color = colors[distrib(gen)];
    
    const int MAX_LEN = 200;

    char name_buffer[MAX_LEN];
    ZeroMemory(name_buffer, MAX_LEN);

    int bytes_received = recv(client_socket, name_buffer, MAX_LEN, 0);
    if (bytes_received > 0) {
        client_list[client_num].name.assign(name_buffer, bytes_received);
        std::cout << client_list[client_num].color << client_list[client_num].name << " has joined the chat." << color_reset << std::endl;
    }
    else {
        std::cerr << "Error receiving name from client." << std::endl;
        closesocket(client_socket);
        remove_client(client_num);
        return;
    }

    while (!exit_flag) {
        ZeroMemory(name_buffer, MAX_LEN);
        int bytes_received = recv(client_socket, name_buffer, MAX_LEN, 0);

        if (bytes_received > 0) {
            std::string msg = client_list[client_num].color + client_list[client_num].name + ": "
                + std::string(name_buffer, 0, bytes_received) + color_reset;
            std::cout << msg << std::endl;
            broadcast_message(msg, client_num);
        }
        else {
            if (bytes_received == 0) {
                std::cout << client_list[client_num].color << client_list[client_num].name << " has left the chat." << color_reset << std::endl;
            }
            else {
                std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            }
            break;
        }
    }

    remove_client(client_num);
}

void Server::broadcast_message(const std::string& msg, int sender_id) {
    std::lock_guard<std::mutex> lock(client_list_mtx);
    for (size_t i = 0; i < client_list.size(); ++i) {
        if (i != sender_id) {
            send(client_list[i].socket, msg.c_str(), static_cast<int>(msg.length()), 0);
        }
    }
}

void Server::remove_client(int id) {
    std::lock_guard<std::mutex> lock(client_list_mtx);
    if (id < client_list.size()) {
        closesocket(client_list[id].socket);
        client_list.erase(client_list.begin() + id);
    }
}

BOOL WINAPI Server::CtrlHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT) {
        exit_flag = true;
        std::cout << "Ctrl+C pressed. Shutting down..." << std::endl;
        return TRUE;
    }
    return FALSE;
}

const std::vector<std::string> Server::colors = {
    "\033[0;31m", // Red
    "\033[0;32m", // Green
    "\033[0;33m", // Yellow
    "\033[0;34m", // Blue
    "\033[0;35m", // Magenta
    "\033[0;36m"  // Cyan
};

const std::string Server::color_reset = "\033[0m";
