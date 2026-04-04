#pragma once

#include <cstdint>
#include <unordered_map>

#include "connection.h"
#include "threadpool.h"

class TcpServer {
    public:
        TcpServer(const char* ip,uint16_t port);
        ~TcpServer();

        void Run();
    
    private:
    void InitEpoll();
    void AddEpoll(int fd,uint32_t events);
    void RemoveEpoll(int fd);
    void HandleAccept();
    void HandleRead(int fd);
    void CloseClient(int fd);
    bool SendWelcomeMessage(int fd);
    
    private:
        int listen_fd_{-1};
        int epoll_fd_{-1};
        std::unordered_map<int, Connection> connections_;
        ThreadPool thread_pool_;
};
