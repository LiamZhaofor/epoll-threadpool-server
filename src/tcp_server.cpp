#include "tcp_server.h"

#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<sys/epoll.h>

#include<cerrno>
#include<cstring>
#include<iostream>
#include<fcntl.h>
#include<string>
#include<cstdlib>

static void Die(const char* msg){
    std::cerr<<msg<<" errno="<<errno<<"("<<std::strerror(errno)<<")\n";
    std::exit(1);
}

static void SetNonBlocking(int fd){
    int flag = ::fcntl(fd,F_GETFL,0);
    if(flag < 0) Die("fcntl(F_GETFL) failed");
    if(::fcntl(fd,F_SETFL,flag | O_NONBLOCK) == -1) Die("fcntl(F_SETFL) failed");
}

bool TcpServer::SendWelcomeMessage(int fd){
    const char* welcome = 
        "\033[1;32m"
        "****************************************\n"
        "*   Welcome to Epoll Threadpool Server  *\n"
        "*   Type 'quit' to leave the server.    *\n"
        "****************************************\n"
        "\033[0m\n"
        ">>";
    if(::send(fd,welcome,std::strlen(welcome),0) < 0){
        std::cerr<<"[server] send welcome message failed fd="<<fd<<"\n";
        return false;
    }
    return true;
}

void TcpServer::InitEpoll(){
    epoll_fd_ = ::epoll_create1(0);
    if(epoll_fd_ < 0) Die("epoll_create1 failed");
}

void TcpServer::AddEpoll(int fd,uint32_t events){
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if(::epoll_ctl(epoll_fd_,EPOLL_CTL_ADD,fd,&ev) < 0) Die("epoll_ctl(ADD) failed");
}

void TcpServer::RemoveEpoll(int fd){
    if(::epoll_ctl(epoll_fd_,EPOLL_CTL_DEL,fd,nullptr) < 0)
    std::cerr << "[server] epoll_ctl del failed for fd=" << fd << '\n';
}

TcpServer::TcpServer(const char* ip,uint16_t port)
    : thread_pool_(4) {  // Initialize thread pool with 4 threads
    listen_fd_ = ::socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd_ < 0) Die("socket failed");
    SetNonBlocking(listen_fd_);     //设置为非阻塞

    int opt = 1;
    if(::setsockopt(listen_fd_,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) < 0){
        Die("setsocketopt(SO_REUSEADDR) failed");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(::inet_pton(AF_INET,ip,&addr.sin_addr) != 1){
        Die("inet_pton failed");
    }
    if(::bind(listen_fd_,reinterpret_cast<sockaddr*>(&addr),sizeof(addr)) < 0){
        Die("bind failed");
    }
    if(::listen(listen_fd_,SOMAXCONN) < 0){
        Die("listen failed");
    }
    std::cout<<"[server] listening on "<<ip<<":"<<port<<'\n';
    InitEpoll();             //建立epoll实例
    AddEpoll(listen_fd_,EPOLLIN);           //监听listen_fd_的可读事件
}

TcpServer::~TcpServer(){
    if(listen_fd_ >= 0 ) ::close(listen_fd_);
    if(epoll_fd_ >= 0) ::close(epoll_fd_);
}

void TcpServer::Run(){
    constexpr int KMaxEvents = 64;
    epoll_event events[KMaxEvents];

    while(true){
        int nfds = ::epoll_wait(epoll_fd_,events,KMaxEvents,-1);
        if(nfds < 0){
            if(errno == EINTR) continue;
            Die("epoll_wait failed");
        }
        for(int i = 0; i < nfds; ++i){
            if(events[i].events & (EPOLLERR | EPOLLHUP)){
                std::cerr<<"[server] epoll error on fd="
                <<events[i].data.fd<<"\n"
                << " EPOLLERR=" << static_cast<bool>(events[i].events & EPOLLERR)
                << " EPOLLHUP=" << static_cast<bool>(events[i].events & EPOLLHUP)
                << "\n";
                CloseClient(events[i].data.fd);
                continue;
            }
            else if(events[i].events & EPOLLIN){
                int fd = events[i].data.fd;
                if(fd == listen_fd_)
                    HandleAccept();
                else 
                    HandleRead(fd);
            }
        }
    }
}


void TcpServer::HandleAccept(){
    while (true)
    {
        sockaddr_in cli{};
        socklen_t len = sizeof(cli);
        int fd = ::accept4(listen_fd_,reinterpret_cast<sockaddr*>(&cli),&len,SOCK_NONBLOCK);
        
        if(fd < 0){
            if(errno == EINTR) continue;
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            Die("accept failed");
        }
        std::cout << "[debug] accepted fd=" << fd << "\n";
//        SetNonBlocking(fd);  //设置为非阻塞
        bool welcome_sent = SendWelcomeMessage(fd);     //发送欢迎消息
        if(!welcome_sent) {             //如果发送欢迎消息失败，关闭连接并继续接受下一个连接
            ::close(fd);
            continue;
        }  
        AddEpoll(fd, EPOLLIN);
        connections_.emplace(fd, Connection(fd));  //创建Connection对象并存储到connections_中
        char ipbuf[INET_ADDRSTRLEN]{};
        ::inet_ntop(AF_INET,&cli.sin_addr,ipbuf,sizeof(ipbuf));
        std::cout<<"[server] client connected: "<<ipbuf<<":"<<ntohs(cli.sin_port)<<"\n";
        
    }
}


        
void TcpServer::HandleRead(int fd){
    char buf[4096];
    std::cout << "[debug] HandleRead fd=" << fd << "\n";

    auto it = connections_.find(fd);
    if(it == connections_.end()){
        std::cerr << "[server] no connection found for fd=" << fd << "\n";
        CloseClient(fd);
        return;
    }

    Connection& conn = it->second;
    ssize_t n = ::recv(fd,buf,sizeof(buf),0);

        if(n > 0){

            conn.ReadBuffer().append(buf, n);

            while(true){
                std::size_t pos = conn.ReadBuffer().find('\n');
                if(pos == std::string::npos) break;  //没有完整的命令
                std::string msg = conn.ReadBuffer().substr(0,pos + 1);  //提取
                conn.ReadBuffer().erase(0,pos + 1);  //删除已处理的部分

                //处理退出逻辑
                while(!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')){
                msg.pop_back();
                }
                if(msg == "quit"){
                    const char* goodbye =
                    "\033[33m"
                    "Bye! Connection will be closed.\n"
                    "\033[0m";

                    if(::send(fd,goodbye,strlen(goodbye),0) < 0){
                        std::cerr<<"[server] send goodbye message failed\n";
                    }
                    std::cout<<"[server] client requested quit\n";
                    CloseClient(fd);
                    return;
                }
            
            std::string response = msg + "\n";
            thread_pool_.Enqueue([fd, response](){
                // Implementation for sending response
                std::cout << "[debug] Sending response to fd=" << fd << ": " << response;

            });

            ssize_t sent = 0;   //正常输出逻辑
            while (sent < static_cast<ssize_t>(response.size()))
            {
                ssize_t m = ::send(fd,response.data() + sent,response.size() - sent,0);
                if(m > 0) sent += m;
                else if( m < 0 && errno == EINTR) continue;
                else if( m < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
                else {
                    std::cerr<<"[server] send error\n";
                    CloseClient(fd);
                    return;
                }
            }

            // const char* prompt = ">>";
            // if(::send(fd,prompt,strlen(prompt),0) < 0){
            //     if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) return;
            //     std::cerr<<"[server] send prompt failed\n";
            //     CloseClient(fd);
            //     return;
            // }
        }
        return;
    }
        else if(n == 0){
            std::cout<<"[server] client disconnected\n";
            CloseClient(fd);
            return;
        }else {
            if(errno == EINTR) return;
            else if(errno == EAGAIN || errno == EWOULDBLOCK) return;
            std::cerr<<"[server] recv error errno="<<errno<<"("<<std::strerror(errno)<<")\n";
            CloseClient(fd);
            return;
        } 
    }


void TcpServer::CloseClient(int fd){
    std::cout << "[debug] CloseClient fd=" << fd << "\n";
    RemoveEpoll(fd);
    ::close(fd);
    
    auto it = connections_.find(fd);
    if(it != connections_.end()){
        connections_.erase(it);
    }
}
