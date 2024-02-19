#include <string>
#include "console.hpp"
#include "gpio.hpp"
#include "server.hpp"
#include "utility.hpp"

bool server_running = false;
int ssock;

void ctrl_c(int) {
    server_running = false;
    phm::info.println("PiHoMi server is shutting down...");
    std::exit(0);
}

int main() {

    phm::info.println("PiHoMi server started at " + phm::ip_address() + ":3141 with PID " + std::to_string(getpid()));

    server_running = true;
    signal(SIGINT, ctrl_c);
    signal(SIGPIPE, SIG_IGN);

    phm::gpio::begin();
    phm::server server(3141, phm::first_acm_device());
    ssock = server.sock();

    phm::register_command("exit", [](const auto&) { ctrl_c(0); });
    phm::register_command("status", [&](const auto& args) { server.print_status(args); });
    phm::register_command("set", [&](const auto& args) { server.set_state(args); });

#ifndef DEBUG
    phm::debug.set_enabled(false);
#endif

    while (server_running) phm::execute_command(phm::in.readln());

    pthread_cancel(server.thread().native_handle());

    return 0;
}
