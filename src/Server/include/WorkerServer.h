#pragma once

#include "include.h"
#include "connection.h"
#include "User_manager.h"
#include "user_page.h"

#define TIME -1

class Connection;

class Worker {
public:
    Worker(int id);
    ~Worker();
    void start();
    void make_queue(int cli);
    void Notify(int from_fd, int to_fd, const std::string& msg);
    void join();
private:
    int make_nonblocking(int sockfd);
    void eventloop();
    void hand_io(int cli);
    void clean_client(int cli);
    void remove_fd(int fd);

    int _id;
    std::vector<int> cli_fds;
    std::thread _thread;
    std::mutex cli_mutex;
    std::unordered_map<int, std::shared_ptr<Connection>> conn;
    UserManager user_msg;
    int epoll_worker;
    bool _running;
};