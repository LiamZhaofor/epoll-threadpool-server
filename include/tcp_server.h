#pragma once

#include <cstdint>
#include <unordered_map>

#include "connection.h"
#include "threadpool.h"

struct TaskResult {
    int fd;
    std::string response;
};

class TcpServer {
    public:
        TcpServer(const char* ip,uint16_t port);
        ~TcpServer();

        void Run();
        void ProcessTaskResults();
    
    private:
    void InitEpoll();
    void AddEpoll(int fd,uint32_t events);
    void RemoveEpoll(int fd);
    void HandleAccept();
    void HandleRead(int fd);
    void CloseClient(int fd);
    bool SendWelcomeMessage(int fd);
    void InitWakeFd();
    void HandleWakeFd();
    
    private:
        int listen_fd_{-1};
        int epoll_fd_{-1};
        int wake_fd_{-1};
        std::unordered_map<int, Connection> connections_;
        ThreadPool thread_pool_;

        std::queue<TaskResult> task_queue_;
        std::mutex task_queue_mutex_;
};

