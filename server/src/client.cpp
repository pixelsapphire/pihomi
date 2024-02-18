#include <algorithm>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "client.hpp"
#include "console.hpp"

phm::client::client(int fd, int epollfd, phm::controller& controller) : _fd(fd), _epoll_fd(epollfd), _controller(controller) {
    epoll_event ee{EPOLLIN | EPOLLRDHUP, {.ptr=this}};
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _fd, &ee);
}

phm::client::~client() {
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _fd, nullptr);
    shutdown(_fd, SHUT_RDWR);
    close(_fd);
}

void phm::client::write(const std::string& msg) {
    ::write(_fd, msg.c_str(), msg.size());
    phm::debug.println("Wrote to " + std::to_string(_fd) + ": " + msg.substr(0, msg.size() - 1) + " (" + std::to_string(msg.size()) + " bytes)");
}

void phm::client::send_current_state() {
    //az;ap;ag;g1;g2;g3;g4;wl;pf;pv;
    std::stringstream ss;
    ss << _controller.get_clock().get_state() << ";";
    ss << _controller.get_irrigation().get_state() << ";";
    ss << _controller.outlets_state() << ";";
    if (_controller.outlets_state())
        for (uint8_t i = 0; i < 4; ++i) ss << _controller.get_outlet(i).get_state() << ";";
    else ss << "0;0;0;0;";
    ss << uint16_t(_controller.get_irrigation().get_water_level()) << ";";
    ss << _controller.get_irrigation().get_watering_delay() << ";";
    ss << _controller.get_irrigation().get_watering_volume() << "\n";
    std::string msg = ss.str();
    write(msg);
}

void phm::client::remove_from(std::vector<client*>& clients) {
    phm::info.println("Client " + std::to_string(_fd) + " disconnected");
    std::erase(clients, this);
    delete this;
}

std::string phm::client::read() {
    char buf[32]{0};
    ::read(_fd, buf, 32);
    std::string buffer = buf;
    if (not buffer.empty())
        phm::debug.println("Read from " + std::to_string(_fd) + ": " + buffer.substr(0, buffer.size() - 1) +
                           " (" + std::to_string(buffer.size()) + " bytes)");
    return buffer;
}

void phm::client::handle_event(uint32_t events, std::vector<client*>& clients, std::array<bool, 4>& outlets) {
    if (events & EPOLLIN) {
        std::string buffer = read();
        if (buffer.empty()) remove_from(clients);
        else {
            handle_message(buffer, outlets);
            send_current_state();
        }
    }
    if (events & ~EPOLLIN and std::find(clients.begin(), clients.end(), this) != clients.end()) remove_from(clients);
}

void phm::client::handle_message(const std::string& msg, std::array<bool, 4>& outlets) {
    if (msg == "o\n") {
        if (_controller.outlets_state()) {
            for (uint8_t i = 0; i < 4; ++i) {
                outlets[i]=_controller.get_outlet(i).get_state();
                _controller.get_outlet(i).set_state(false);
            }
        } else for (uint8_t i = 0; i < 4; ++i) _controller.get_outlet(i).set_state(outlets[i]);
        _controller.set_outlets_state(!_controller.outlets_state());
    }
    else if (_controller.outlets_state()) {
        if(msg == "o1\n") _controller.get_outlet(0).toggle();
        else if(msg == "o2\n") _controller.get_outlet(1).toggle();
        else if(msg == "o3\n") _controller.get_outlet(2).toggle();
        else if(msg == "o4\n") _controller.get_outlet(3).toggle();
    }
    if (msg == "c\n") _controller.get_clock().set_state(!_controller.get_clock().get_state());
    else if (msg == "i\n") _controller.get_irrigation().set_state(!_controller.get_irrigation().get_state());
    else if (msg[0] == 'f') _controller.get_irrigation().set_watering_delay(std::stof(msg.substr(1,msg.find('\n'))));
    else if (msg[0] == 'v') _controller.get_irrigation().set_watering_volume(std::stof(msg.substr(1,msg.find('\n'))));
}
