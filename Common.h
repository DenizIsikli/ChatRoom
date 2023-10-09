#ifndef CHATROOM_COMMON_H
#define CHATROOM_COMMON_H

#include <string>
#include <vector>
#include <WinSock2.h>
#include <thread>
#include <mutex>

using namespace std;

#define MAX_LEN 200
#define NUM_COLORS 6

// Both files
extern const string default_color;
const string default_color = "\033[0m";
string colors[]={"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
string color(int code);
string color(int code) { return colors[code % NUM_COLORS]; }
void handle_error(const char* message);

// Server
struct terminal {
    int id;
    string name;
    SOCKET socket;
    thread th;
};

extern vector<terminal> clients;
extern int seed;
extern mutex cout_mtx, clients_mtx;

void set_name(int id, char name[]);
void shared_print(string str);
void broadcast_message(string message, int sender_id);
void end_connection(int id);



// Client
extern bool exit_flag;
extern int client_socket;

void CtrlHandler();
int eraseText(int cnt);
void send_message();
void recv_message();

#endif //CHATROOM_COMMON_H
