#ifndef PIHOMI_CLIENT_HPP
#define PIHOMI_CLIENT_HPP

#include <vector>
#include "controller.hpp"

namespace phm {

    class client {
    
        int _fd;
        int _epoll_fd;
        int connected;
        phm::controller& _controller;

    public:

        client(int fd, int epollfd, phm::controller& controller);

        ~client();

        void write(const std::string& msg);

        [[nodiscard]] std::string read();

        void remove_from(std::vector<client*>& clients);

        void handle_event(uint32_t events, std::vector<client*>& clients, std::array<bool, 4>& outlets);

        void handle_message(const std::string& msg, std::array<bool,4>& outlets);

        void send_current_state();
    };
}

#endif // PIHOMI_CLIENT_HPP