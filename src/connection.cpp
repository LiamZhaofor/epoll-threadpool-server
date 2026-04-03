#include "connection.h"

Connection::Connection(int fd):fd_(fd){}

int Connection::Fd() const{
    return fd_;
}

std::string& Connection::ReadBuffer(){
    return read_buffer_;
}

std::string& Connection::WriteBuffer(){
    return write_buffer_;
}