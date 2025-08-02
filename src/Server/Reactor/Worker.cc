#include "WorkerServer.h"

Worker::Worker(int id)
: epoll_worker(-1),
_id(id)
{}

Worker::~Worker() {
    _running = false;

    if(epoll_worker >= 0) {
        close(epoll_worker);
    }

    if(_thread.joinable()) {
        _thread.join();
    }

    conn.clear();
}

void Worker::make_queue(int cli){

    std::lock_guard<std::mutex> lock(cli_mutex);
    if(make_nonblocking(cli) == -1){
        perror("Cli_make_nonblocking error");
        return;
    }

    struct epoll_event ev;
    ev.data.fd = cli;
    ev.events = EPOLLIN | EPOLLET;
    conn[cli] = std::make_shared<Connection>(cli, user_msg);

    if(epoll_ctl(epoll_worker, EPOLL_CTL_ADD, cli, &ev) < 0) {
        std::cerr << "epoll_ctl_add_error in Worker.cc" << std::strerror(errno) << std::endl;
    }

    std::cout << "\033[1;33m" << _id << "id\033[0m" << std::endl;
    std::cout << "\033[1;33m" << cli << "fd\033[0m" << std::endl;
    cli_fds.push_back(cli);
}

void Worker::start() {
    epoll_worker = epoll_create1(0);
    if(epoll_worker == -1) exit(1);
    _thread = std::thread([this](){this->eventloop();});
}

int Worker::make_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Worker::eventloop() {
    _running = true;
    struct epoll_event events[NUM];
    while(_running) {
        int n = epoll_wait(epoll_worker, events, NUM, TIME);

        if(n < 0) {
            if(errno == EINTR) continue;
        }

        for(int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            
            if(events[i].events & (EPOLLERR | EPOLLHUP) || !(events[i].events & EPOLLIN)) {
                remove_fd(fd);
                continue;
            }
            std::cout << fd << "fd" << std::endl;
            std::cout << _id << "id" << std::endl;
            auto it = conn.find(fd);
            if(it == conn.end()) {
                std::cerr << "Invalid fd or nullptr connection: " << fd << std::endl;
                remove_fd(fd);
                continue;
            }
            if(!conn[fd]->readMessage()) {
                remove_fd(fd);
            }

            // 处理到来的客户端请求~
        }
    }
}

void Worker::remove_fd(int fd) {

    std::lock_guard<std::mutex> lock(cli_mutex);
    epoll_ctl(epoll_worker, EPOLL_CTL_DEL, fd, nullptr);
    conn.erase(fd);
    cli_fds.erase(std::remove(cli_fds.begin(), cli_fds.end(), fd), cli_fds.end());
    std::cout << "拜拜" << std::endl;
    user_msg.removeUser(fd);
    close(fd);
}

void Worker::Notify(int from_fd, int to_fd, const std::string& msg) {
    if(conn.count(to_fd)) {
        conn[to_fd]->sendMessage(from_fd, msg);
    }
}


