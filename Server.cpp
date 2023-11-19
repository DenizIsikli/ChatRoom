#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <random>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_LEN 200
#define NUM_COLORS 6

void handle_error(const char* message);
void set_name(int id, char name[]);
void shared_print(string str);
void broadcast_message(string message, int sender_id);
void end_connection(int id);
void handle_client(SOCKET client_socket_, int id);
int run_server();

struct terminal {
    int id;
    string name;
    SOCKET socket;
    thread th;
};

const vector<string> client_colors = {
        "\033[31m",  // Red
        "\033[32m",  // Green
        "\033[33m",  // Yellow
        "\033[34m",  // Blue
        "\033[35m",  // Purple
        "\033[36m"   // Cyan
};

vector<terminal> clients;
int seed = 0;
bool serverRunning = true;
mutex cout_mtx, clients_mtx;

void handle_error(const char* message) {
    cerr << message << endl;
    cerr << "Error code: " << WSAGetLastError() << endl;
    WSACleanup();
    exit(0);
}

void set_name(int id, char name[]) {
    for (auto& client_ : clients) {
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

    for (auto& client_ : clients) {
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

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> color_dist(0,  client_colors.size() - 1);
    string client_color = client_colors[color_dist(gen)];

    shared_print(client_color + name + string(" has joined the chat.\n"));

    while (true) {
        char message[MAX_LEN];
        int bytes_received = recv(client_socket_, message, MAX_LEN, 0);
        if (bytes_received <= 0) {
            return;
        }
        message[bytes_received] = '\0'; // Null-terminate the message
        if (strcmp(message, "exit") == 0) {
            shared_print(client_color + name + string(" has left the chat.\n"));
            end_connection(id);
            return;
        }

        broadcast_message(client_color + name + string(" : ") + message, id);
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

    cout << "\n\t  ====== Welcome to the chat-room ======\n";

    sockaddr_in client{};
    int client_len = sizeof(client);

    while (serverRunning) {
        SOCKET client_socket_ = accept(server_socket, (sockaddr*)&client, &client_len);
        if (client_socket_ == INVALID_SOCKET) {
            handle_error("Accept() failed");
        }

        seed++;
        thread(handle_client, client_socket_, seed).detach();
        clients.push_back({seed, string("Anonymous"), client_socket_});
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
