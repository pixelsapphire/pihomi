#include "server.hpp"


void setReuseAddr(int sock)
{
    const int one = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(res) error(1,errno, "setsockopt failed");
}

Server::Server(uint port,std::string serial): devicecontroller(DeviceController(serial))
{
    _epollFd = epoll_create1(0);
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if(_sock == -1) 
        error(1, errno, "socket failed");

    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons((short)port), .sin_addr={INADDR_ANY},.sin_zero={0}};
    int res = bind(_sock, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if(res) 
        error(1, errno, "bind failed");
    
    res = listen(_sock, 1);
    if(res) 
        error(1, errno, "listen failed");
    epoll_event ee {EPOLLIN, {.ptr=this}};
    epoll_ctl(_epollFd, EPOLL_CTL_ADD, _sock, &ee);
    serverThread = std::thread(&Server::serverLoop,this);
}

Server::~Server()
{
    close(_sock);
    if(serverThread.joinable())
    {
        serverThread.join();
    }
}

int Server::sock() const
{
    return _sock;
}

void Server::handleEvent(uint32_t events) 
{
    if(events & EPOLLIN)
    {
        sockaddr_in clientAddr{};
        socklen_t clientAddrSize = sizeof(clientAddr);
        
        auto clientFd = accept(_sock, (sockaddr*) &clientAddr, &clientAddrSize);
        if(clientFd == -1) error(1, errno, "accept failed");
        
        std::cout << "new connection from: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " (fd: "<< clientFd <<")" << std::endl;
        close(clientFd);
    }
    if(events & ~EPOLLIN){
        error(0, errno, "Event %x on server socket", events);
        close(_sock);
        serverRunning=false;
        
    }
}


void Server::serverLoop()
{
    epoll_event ee;
    while(serverRunning)
    {
        int n = epoll_wait(_epollFd, &ee, 1, -1);
        if(n==-1) 
        {
            error(0,errno,"epoll_wait failed");
            ctrl_c(SIGINT);
            exit(1);
        }
        handleEvent(ee.events);
    }
}