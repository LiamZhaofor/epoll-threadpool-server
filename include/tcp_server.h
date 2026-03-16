#pragma once
#include <cstdint>

class TcpServer {
    public:
        TcpServer(const char* ip,uint16_t port);
        ~TcpServer();

        void RunBlockingServer();
    
    private:
    int listen_fd_{-1};
};
