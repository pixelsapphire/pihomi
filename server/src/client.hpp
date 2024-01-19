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

struct Client
{
    int fd;
    int connected;
    int timeoutCounter;
    std::thread clientThread;
    Client();
    ~Client();
    void updateClient();
    void checkConnection();
};

