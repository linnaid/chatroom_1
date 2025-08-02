#pragma once

#include "include.h"
#include "user_chat.pb.h"
#include "protocol.h"

extern std::atomic<bool> running;

class Heart {
public:
    Heart();
    ~Heart();
    void init();
private:
    void send_heart();

    int _sockfd;
    int _port;
    std::string _ip;
    sockaddr_in _fd;
};