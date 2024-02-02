#include <string>
#include "console.hpp"
#include "gpio.hpp"
#include "server.hpp"

bool server_running = false;
int ssock;

void ctrl_c(int) {
    server_running = false;
    phm::info.println("PiHoMi server is shutting down...");
    std::exit(0);
}

int main(int argc, char** argv) {
    phm::info.println("PiHoMi server started: \"pihomi " + phm::to_string(argc - 1, argv + 1) + "\" with PID " + std::to_string(getpid()));
    if (argc < 2) {
        phm::error.println("Serial port device has to be specified.");
        ctrl_c(0);
    }
    server_running = true;
    signal(SIGINT, ctrl_c);
    signal(SIGPIPE, SIG_IGN);

    phm::gpio::begin();
    phm::server server(3141, argv[1]);
    ssock = server.sock();

    phm::register_command("exit", [](const auto&) { ctrl_c(0); });
    phm::register_command("status", [&](const auto& args) { server.print_status(args); });
    phm::register_command("set", [&](const auto& args) { server.set_state(args); });

    while (server_running) phm::execute_command(phm::input.readln());

    pthread_cancel(server.thread().native_handle());

    return 0;
}
