#include "messageCenter.h"

// 单例实现
MessageCenter& MessageCenter::instance() {
    static MessageCenter instance;
    return instance;
}

void MessageCenter::register_worker(int id, std::shared_ptr<Worker> worker) {
    workers[id] = worker;
}

// 这里是对好友间聊天的处理～
void MessageCenter::dispatch(int from_fd, int to_fd, const std::string& msg){
    auto it = workers.find(Worker_fd[to_fd]);
    if(it != workers.end()){
        it->second->Notify(from_fd, to_fd, msg);
    } else {
        std::cout << "Not Find This User." << std::endl;
    }
}

// 把群里的消息发给每个群有，我还没实现群，先不写啦～
void MessageCenter::broadcast_group(int from_fd, const std::string& group_name, const std::string& msg){
    
}

