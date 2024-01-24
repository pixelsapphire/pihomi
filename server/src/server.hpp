#ifndef PIHOMI_SERVER_HPP
#define PIHOMI_SERVER_HPP

#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "client.hpp"
#include "controller.hpp"

extern bool server_running;
extern void ctrl_c(int);

namespace phm {

    class server {

        int _sock;
        int _epoll_fd;
        std::array<bool, 4> socks;
        phm::controller controller;
        std::thread server_thread;
        std::vector<client*> clients;

        static void set_reuse_addr(int sock);

    public:

        server(uint32_t port,std::string serial);

        ~server();

        [[nodiscard]] int sock() const;

        [[nodiscard]] std::thread& thread();

        void handle_event(uint32_t events);

        void server_loop();

        void inter();

        void print_status(const std::vector<std::string>& args) const;

        void set_state(const std::vector<std::string>& args);
    };
}

#endif // PIHOMI_SERVER_HPP
