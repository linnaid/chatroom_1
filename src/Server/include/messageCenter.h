#pragma once

#include "include.h"
#include "WorkerServer.h"
#include "user_page.h"
#include "MasterServer.h"

class Worker;

class MessageCenter {
public:
    // 单例模式
    static MessageCenter& instance();

    void register_worker(int id, std::shared_ptr<Worker> worker);
    void dispatch(int from_fd, int to_fd, const std::string& msg);
    void broadcast_group(int from_fd, const std::string& group_name, const std::string& msg);

private:
    std::unordered_map<int, std::shared_ptr<Worker>> workers;
};

