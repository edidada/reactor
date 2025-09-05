#include <iostream>
#include "EventLoop.h"
#include "TcpConn.h"
#include "TcpServer.h"

int main() {
    EventLoop evloop;
    TcpServer server(evloop);

    server.Start(8989, [](TcpConn::Ptr conn) {
        std::cout << "New connection established\n";

        // conn->SetReadCallback([conn]() {
        //     std::cout << "Received: " << conn->GetAllData() << std::endl;
        //     conn->Send("HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello Mark!\r\n", 53);
        // });
        conn->SetReadCallback([conn]() {
            std::cout << "Received: " << conn->GetDataUntilCrLf() << std::endl;
            conn->Send("Hello World!\r\n", 15);
        });
    });

    evloop.Run();
    return 0;
}