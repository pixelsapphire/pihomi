#include "server.hpp"


void phm::server::set_reuse_addr(int sock) {
    const int one = 1;
    const int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (res) error(1, errno, "setsockopt failed");
}

phm::server::server(uint32_t port,std::string serial): controller(serial) {

    for(uint8_t i = 0; i < 4; ++i) socks[i] = controller.get_outlet(i).get_state();
    _epollFd = epoll_create1(0);
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock == -1) error(1, errno, "socket failed");

    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons(short(port)), .sin_addr={INADDR_ANY}, .sin_zero={0}};
    set_reuse_addr(_sock);
    int res = bind(_sock, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if (res) error(1, errno, "bind failed");

    res = listen(_sock, 1);
    if(res) error(1, errno, "listen failed");
    epoll_event ee{EPOLLIN, {.ptr=this }};
    epoll_ctl(_epollFd, EPOLL_CTL_ADD, _sock, &ee);
    server_thread = std::thread(&phm::server::server_loop, this);
}

phm::server::~server() {
    close(_sock);
    if (server_thread.joinable()) server_thread.join();
}

int phm::server::sock() const { return _sock; }

std::thread& phm::server::thread() { return server_thread; }

void phm::server::handle_event(uint32_t events) {

    if (events & EPOLLIN) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrSize = sizeof(clientAddr);

        auto clientFd = accept(_sock, (sockaddr*) &clientAddr, &clientAddrSize);
        if (clientFd == -1) error(1, errno, "accept failed");

        std::cout << "new connection from: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " (fd: "<< clientFd <<")" << std::endl;
        clients.push_back(new Client(clientFd, _epollFd, controller));
    }
    if(events & ~EPOLLIN) {
        error(0, errno, "Event %x on server socket", events);
        close(_sock);
        server_running=false;
    }
}

void phm::server::inter() { raise(SIGINT); }

void phm::server::server_loop() {
    epoll_event ee;
    while(server_running) {
        int n = epoll_wait(_epollFd, &ee, 1, -1);
        if(n == -1) {
            error(0,errno,"epoll_wait failed");
            ctrl_c(SIGINT);
            exit(1);
        }
        if (ee.data.ptr == this) handle_event(ee.events);
        else ((Client*) ee.data.ptr)->handleEvent(ee.events, clients, socks);
    }
}
