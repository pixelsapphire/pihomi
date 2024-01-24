#include "console.hpp"
#include "server.hpp"

void phm::server::set_reuse_addr(int sock) {
    const int one = 1;
    const int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (res) phm::error(1, errno, "setsockopt failed");
}

phm::server::server(uint32_t port,std::string serial): controller(serial) {

    for (uint8_t i = 0; i < 4; ++i) socks[i] = controller.get_outlet(i).get_state();
    _epoll_fd = epoll_create1(0);
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock == -1) phm::error(1, errno, "socket failed");

    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons(short(port)), .sin_addr={INADDR_ANY}, .sin_zero={0}};
    set_reuse_addr(_sock);
    int res = bind(_sock, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if (res) phm::error(1, errno, "bind failed");

    res = listen(_sock, 1);
    if (res) phm::error(1, errno, "listen failed");
    epoll_event ee{EPOLLIN, {.ptr=this }};
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _sock, &ee);
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
        sockaddr_in client_address{};
        socklen_t client_addressSize = sizeof(client_address);

        auto client_fd = accept(_sock, (sockaddr*) &client_address, &client_addressSize);
        if (client_fd == -1) phm::error(1, errno, "accept failed");

        phm::info.println("new connection from: " + std::string(inet_ntoa(client_address.sin_addr)) + ":" + std::to_string(ntohs(client_address.sin_port)) + " (fd: " + std::to_string(client_fd) + ")");
        clients.push_back(new client(client_fd, _epoll_fd, controller));
    }
    if (events & ~EPOLLIN) {
        phm::error(0, errno, "Event %x on server socket", events);
        close(_sock);
        server_running=false;
    }
}

void phm::server::inter() { raise(SIGINT); }

void phm::server::server_loop() {
    epoll_event ee;
    while (server_running) {
        int n = epoll_wait(_epoll_fd, &ee, 1, -1);
        if (n == -1) {
            phm::error(0, errno, "epoll_wait failed");
            ctrl_c(SIGINT);
            exit(1);
        }
        if (ee.data.ptr == this) handle_event(ee.events);
        else ((Client*) ee.data.ptr)->handleEvent(ee.events, clients, socks);
    }
}

void phm::server::print_status(const std::vector<std::string>& args) const {
    if (args.empty() or args[0] == "-h" or args[0] == "--help") {
        (args.empty() ? phm::error : phm::info).println("Usage: status <all|clock|irrigation|outlets>");
        return;
    }
    if (args[0] == "all")
        phm::info.println(std::string("\nSmart home services:") +
                        "\n * clock: " + controller.get_clock().status_str() +
                        "\n * irrigation: " + controller.get_irrigation().status_str() +
                        "\n * outlets: " + controller.outlets_status_str()
        );
    else if (args[0] == "clock") phm::info.println(controller.get_clock().status_str());
    else if (args[0] == "irrigation") phm::info.println(controller.get_irrigation().status_str());
    else if (args[0] == "outlets") phm::info.println(controller.outlets_status_str());
    else phm::error.println("Invalid argument: " + args[0]);
}