#include "tcp_server.h"

#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>

#include<cerrno>
#include<cstring>
#include<iostream>


static void Die(const char* msg){
    std::cerr<<msg<<" errno="<<errno<<"("<<std::strerror(errno)<<")\n";
    std::exit(1);
}

TcpSever::TcpSever(const char* ip,uint16_t port){
    listen_fd_ = ::socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd_ < 0) Die("socket failed");

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

TcpSever::~TcpSever(){
    if(listen_fd_ >= 0 ) ::close(listen_fd_);
}

void TcpSever::RunBlockingEcho(){
    while(true){
        sockaddr_in cli{};
        socklen_t len = sizeof(cli);
        int cfd = ::accept(listen_fd_,reinterpret_cast<sockaddr*>(&cli),&len);
        if(cfd < 0){
            if(errno == EINTR) continue;
            Die("accept failed");
        }

        char ipbuf[INET_ADDRSTRLEN]{};
        ::inet_ntop(AF_INET,&cli.sin_addr,ipbuf,sizeof(ipbuf));
        std::cout<<"[server] client connected: "<<ipbuf<<":"<<ntohs(cli.sin_port)<<"\n";

        char buf[4096];
        while (true)
        {
            ssize_t n = ::recv(cfd,buf,sizeof(buf),0);
            if(n > 0){
                ssize_t sent = 0;
                while (sent < n)
                {
                    ssize_t m = ::send(cfd,buf + sent,n -sent,0);
                    if(m > 0) sent += m;
                    else if(m < 0 && errno == EINTR) continue;
                    else {std::cerr<<"[server] send error\n";break;}
                }
            }else if(n == 0){
                std::cout<<"[server] client disconnected\n";
                break;
            }else {
                if(errno == EINTR) continue;
                std::cerr<<"[server] recv error errno="<<errno<<"("<<std::strerror(errno)<<")\n";
                break;
            }
        }
        
        ::close(cfd);
    }
}