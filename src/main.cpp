#include "tcp_server.h"

int main(){
    TcpServer server("0.0.0.0",8080);
    server.Run();
    return 0;
}