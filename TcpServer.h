#pragma once

#include <functional>
#include <memory>

class TcpConn;
class EventLoop;
class TcpServer {
public:
    using NewConnCallback = std::function<void(std::shared_ptr<TcpConn>)>;
    TcpServer(EventLoop& evloop);
    ~TcpServer();

    void Start(int port, NewConnCallback cb);
private:
    void HandleAccept(int events);

    int listen_fd_;
    EventLoop& evloop_;
    NewConnCallback new_conn_cb_;
    std::function<void(int)> accept_handler_;
};