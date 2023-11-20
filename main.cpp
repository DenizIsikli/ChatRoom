#include <thread>

using namespace std;

extern int run_server();
extern int run_client();

int main() {
    thread server_thread(run_server);
    thread client_thread(run_client);

    server_thread.join();
    client_thread.join();

    return 0;
}
