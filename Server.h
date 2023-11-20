#ifndef SERVER_H
#define SERVER_H

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

#endif // SERVER_H
