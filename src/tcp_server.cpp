#include "tcp_server.h"

#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>

#include<cerrno>
#include<cstring>
#include<iostream>
#include<fcntl.h>
#include<string>

static void Die(const char* msg){
    std::cerr<<msg<<" errno="<<errno<<"("<<std::strerror(errno)<<")\n";
    std::exit(1);
}

static void SetNonBlocking(int fd){
    int flag = ::fcntl(fd,F_GETFL,0);
    if(flag < 0) Die("fcntl(F_GETFL) failed");
    if(::fcntl(fd,F_SETFL,flag | O_NONBLOCK) == -1) Die("fcntl(F_SETFL) failed");
}

TcpServer::TcpServer(const char* ip,uint16_t port){
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
    if(::listen(listen_fd_,128) < 0){
        Die("listen failed");
    }
    std::cout<<"[server] listening on "<<ip<<":"<<port<<'\n';
}

TcpServer::~TcpServer(){
    if(listen_fd_ >= 0 ) ::close(listen_fd_);
}

void TcpServer::RunBlockingServer(){
    while(true){
        sockaddr_in cli{};
        socklen_t len = sizeof(cli);
        int cfd = ::accept(listen_fd_,reinterpret_cast<sockaddr*>(&cli),&len);
        if(cfd < 0){
            if(errno == EINTR) continue;
            if(errno == EAGAIN || errno == EWOULDBLOCK) continue;
            Die("accept failed");
        }
        SetNonBlocking(cfd);  //设置为非阻塞

        char ipbuf[INET_ADDRSTRLEN]{};
        ::inet_ntop(AF_INET,&cli.sin_addr,ipbuf,sizeof(ipbuf));
        std::cout<<"[server] client connected: "<<ipbuf<<":"<<ntohs(cli.sin_port)<<"\n";
        const char* welcome = 
        "\033[1;32m"
        "****************************************\n"
        "*   Welcome to Epoll Threadpool Server  *\n"
        "*   Type 'quit' to leave the server.    *\n"
        "****************************************\n"
        "\033[0m\n"
        ">>";
        if(::send(cfd,welcome,std::strlen(welcome),0) < 0){
            std::cerr<<"[server] send welcome message failed\n";
            ::close(cfd);
            continue;
        }
        char buf[4096];

        while (true)                            //逻辑处理循环
        {
            ssize_t n = ::recv(cfd,buf,sizeof(buf),0);
            if(n > 0){
                std::string msg(buf,n);  //处理退出逻辑
                while(!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')){
                    msg.pop_back();
                }
                if(msg == "quit"){
                    const char* goodbye =
                    "\033[33m"
                    "Bye! Connection will be closed.\n"
                    "\033[0m";

                    if(::send(cfd,goodbye,strlen(goodbye),0) < 0){
                        std::cerr<<"[server] send goodbye message failed\n";
                    }
                    std::cout<<"[server] client requested quit\n";
                    break;
                }

                ssize_t sent = 0;   //正常输出逻辑
                while (sent < n)
                {
                    const char* prompt = ">>";
                    ssize_t m = ::send(cfd,buf + sent,n -sent,0);
                    send(cfd,prompt,strlen(prompt),0);
                    if(m > 0) sent += m;
                    else if(m < 0 && errno == EINTR) continue;
                    else if(m < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
                    else {std::cerr<<"[server] send error\n";break;}
                }
            }else if(n == 0){
                std::cout<<"[server] client disconnected\n";
                break;
            }else {
                if(errno == EINTR) continue;
                else if(errno == EAGAIN || EWOULDBLOCK) continue;
                std::cerr<<"[server] recv error errno="<<errno<<"("<<std::strerror(errno)<<")\n";
                break;
            }
        }
        ::close(cfd);
    }
}