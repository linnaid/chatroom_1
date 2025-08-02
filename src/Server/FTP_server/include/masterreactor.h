#pragma once

#include "include.h"
#include "workerreactor.h"

#define NUM 8
#define MAX_EVENT 20
#define MAX_NUM 1024

class worker;

class master
{
public:
    master(int port = 2100);
    ~master();
    void init();
    void run();
    void distribute_worker(int *cli_fd);

private:
    int make_nonblocking(int sockfd);
    void accept_client();

    int _port;
    int cli_fd;
    int _worker;
    int epoll_master;
    int _time;
    int listen_sockfd;
    std::vector<std::unique_ptr<worker>> workers;
};