#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unordered_set>
#include <list>
#include <signal.h>
#include <thread>
#include <vector>
#include <sstream>

#include "controller.hpp"
//#include <algorithm>


struct Client
{
    int _fd;
    int _epollFd;
    int connected;
    phm::controller &_controller;
    Client(int fd,int epollfd,phm::controller &controller);
    ~Client();
    void write(std::string msg);
    std::string read();
    void remove(std::vector<Client*> &clients);
    void handleEvent(uint32_t events,std::vector<Client*> &clients,std::array<bool,4> &outlets);
    void handleMessage(std::string&msg,std::array<bool,4> &outlets);
    void sendCurrentState();
};

