#include "tcp_server.h"

int main(){
    TcpSever server("0.0.0.0",8080);
    server.RunBlockingServer();
    return 0;
}