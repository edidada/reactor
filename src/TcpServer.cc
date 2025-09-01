#include "../TcpServer.h"
#include "EventLoop.h"
#include "TcpConn.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

TcpServer::TcpServer(EventLoop& evloop)
    : listen_fd_(-1), evloop_(evloop) 
{
}

TcpServer::~TcpServer() {
    if (listen_fd_ != -1) {
        evloop_.DelEvent(listen_fd_);
        close(listen_fd_);
    }
}

void TcpServer::Start(int port, NewConnCallback cb) {
    new_conn_cb_ = cb;
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ == -1) {
        std::cerr << "socket error:" << errno << std::endl;
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int opt = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "setsockopt error: " << errno << std::endl;
        close(listen_fd_);
        return;
    }
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "setsockopt error: " << errno << std::endl;
        close(listen_fd_);
        return;
    }

    if (::bind(listen_fd_, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        std::cerr << "bind error: " << errno << std::endl;
        close(listen_fd_);
        return;
    }

    if (::listen(listen_fd_, SOMAXCONN) == -1)
    {
        std::cerr << "listen error: " << errno << std::endl;
        close(listen_fd_);
        return;
    }

    accept_handler_ = [this](int events){ HandleAccept(events); };
    evloop_.AddEvent(listen_fd_, EPOLLIN, &accept_handler_);
    std::cout << "Server started on port " << port << std::endl;

}

void TcpServer::HandleAccept(int events)
{
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);
    int conn_fd = ::accept4(listen_fd_, (sockaddr *)&client_addr, &len, SOCK_NONBLOCK);
    if (conn_fd == -1)
        return;

    auto conn = std::make_shared<TcpConn>(conn_fd, evloop_);
    if (new_conn_cb_)
        new_conn_cb_(conn);
}