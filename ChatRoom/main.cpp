#include "Server.h"
#include "Client.h"
#include "GlobalConstants.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <random>

#pragma comment(lib, "ws2_32.lib")

int main() {
    try {
        Server serverInstance;
        std::thread server_thread(&Server::run, &serverInstance);

        std::vector<std::unique_ptr<Client>> clients;
        std::vector<std::thread> clientThreads;

        for (int i = 0; i < 1; ++i) {
            auto client = std::make_unique<Client>(SERVER_IP, SERVER_PORT);
            clientThreads.emplace_back(&Client::run, client.get());
            clients.push_back(std::move(client));
        }

        server_thread.join();

        for (auto& thread : clientThreads) {
            thread.join();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
    }

    return 0;
}
