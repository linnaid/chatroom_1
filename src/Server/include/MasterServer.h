#pragma once

#include "include.h"
#include "user_chat.pb.h"
#include "RedisClient.h"
#include "WorkerServer.h"
#include "user_page.h"
#include "messageCenter.h"

extern std::unordered_map<int, int> Worker_fd;

class Worker;

class Master{
public:
    Master();
    ~Master();
    void init();
    void run();
    
private:
    void distribute_worker(int cli_fd);
    int make_nonblocking(int sockfd);
    void accept_client();

    RedisClient _redis;
    int _port;
    int _worker;
    //////////////////////////// 这里也可以有IP
    std::string _ip;
    int epoll_master;
    int listen_sockfd;
    std::vector<std::shared_ptr<Worker>> workers;
};