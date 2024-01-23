#include <string>
#include "server.hpp"

bool serverRunning = false;
int ssock;

void ctrl_c(int) {
    serverRunning = false;
    std::exit(0);
}

#include <wiringSerial.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Serial port has to be specified.\n";
        return -1;
    }
    serverRunning = true;
    signal(SIGINT, ctrl_c);
    Server server(3141, argv[1]);
    
    ssock = server.sock();
    while (serverRunning) {}
    pthread_cancel(server.serverThread.native_handle());

    return 0;
}
