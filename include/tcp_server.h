#pragma once
#include <cstdint>

class TcpSever {
    public:
        TcpSever(const char* ip,uint16_t port);
        ~TcpSever();

        void RunBlockingEcho();
    
    private:
    int listen_fd_{-1};
};
