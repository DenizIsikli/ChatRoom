#ifndef CLIENT_H
#define CLIENT_H

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

extern vector<string> names;
extern bool exit_flag;
extern thread t_send, t_recv;
extern int client_socket;

void CtrlHandler();
void send_message();
void recv_message();
int run_client();

#endif // CLIENT_H
