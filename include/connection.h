#pragma once

#include <string>

class Connection{
    public:
    explicit Connection(int fd);
    int Fd() const;
    std::string& ReadBuffer();
    std::string& WriteBuffer();


    private:
    int fd_{-1};
    std::string read_buffer_;
    std::string write_buffer_;
};