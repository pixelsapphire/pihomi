#include <iostream>
#include <string>
#include "gpio.hpp"
#include "server.hpp"

bool server_running = false;
int ssock;

void ctrl_c(int) {
    server_running = false;
    std::exit(0);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Serial port device has to be specified.\n";
        return -1;
    }
    server_running = true;
    signal(SIGINT, ctrl_c);

    phm::gpio::begin();
    phm::server server(3141, argv[1]);

    ssock = server.sock();
    while (server_running) {}
    pthread_cancel(server.thread().native_handle());

    return 0;
}
