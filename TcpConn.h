#pragma once

#include "MessageBuffer.h"
#include <functional>
#include <memory>
#include <string>

class EventLoop;
class MessageBuffer;
class TcpConn {
public:
    using ReadCallback = std::function<void()>;
    using Ptr = std::shared_ptr<TcpConn>;
    TcpConn(int fd, EventLoop& evloop);
    ~TcpConn();

    void SetReadCallback(ReadCallback cb) {
        read_cb_ = std::move(cb);
    }

    std::string GetAllData();
    std::string GetDataUntilCrLf();

    int Send(const char* buf, size_t len);
private:
    static void SetNonBlocking(int fd);
    void Close();

    void HandleRead();
    void HandleWrite();
    void HandleIO(int events);

    void EnableWrite();
    void DisableWrite();

private:
    int fd_;
    EventLoop& evloop_;
    std::string output_buffer_;
    MessageBuffer input_buffer_;
    bool closed_;
    std::function<void(int)> io_handler_;
    ReadCallback read_cb_;
};