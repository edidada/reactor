# c++ reactor

EventLoop
MessageBuffer

class EventLoop;
class MessageBuffer;
TcpConn

class TcpConn;
class EventLoop;
TcpServer

## EventLoop
    int epfd_;

## MessageBuffer

    std::vector<uint8_t> buffer_;
    std::size_t rpos_;
    std::size_t wpos_;
