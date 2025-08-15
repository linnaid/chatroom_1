#include "workerreactor.h"


worker::worker():
    epoll_worker(-1),
    _time(3000),
    _running(false)
    {}

worker::~worker() {
    _running = false;
    
    if(epoll_worker >= 0) {
        close(epoll_worker);
    }

    if(_thread.joinable()) {
        _thread.join();
    }
}

void worker::start()
{
    epoll_worker = epoll_create1(0);
    // 1
    // std::cout << epoll_worker << "_start" << std::endl;
    if(epoll_worker == -1) exit(1);
    _thread = std::thread([this](){this->event_loop();});

}

int worker::make_nonblocking(int cli)
{
    int flags = fcntl(cli, F_GETFL);
    if(flags == -1) return -1;
    return fcntl(cli, F_SETFL, flags | O_NONBLOCK);
}

void worker::make_queue(int *cli)
{
    std::lock_guard<std::mutex> lock(cli_mutex);
    if(make_nonblocking(*cli) == -1)
    perror("CLI_make_nonblock error");
    struct epoll_event event;
    event.data.fd = *cli;
    event.events = EPOLLIN;
    if(epoll_ctl(epoll_worker, EPOLL_CTL_ADD, *cli, &event) < 0)
    perror("CLI_epoll_add error");
    cli_fd.push_back(*cli);
    // _queue.Push(*cli);

    connections[*cli] = std::make_shared<FTPconnect>(*cli);
    // std::cout << "queue" << std::endl;
    // std::cout << *cli << std::endl;
}



void worker::event_loop()
{
    _running = true;
    int num = 8;
    struct epoll_event events[num];
    while(_running)
    {
        int n = epoll_wait(epoll_worker, events, num, _time);
        if(n < 0)
        {
            if(errno == EINTR)     continue;
        }
        for(int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            // std::cout << fd << "event_loop" << std::endl;
            hand_io(fd);
        }
        // if(cli_fd.empty())
        // _running = false;
    }
}

void worker::hand_io(int cli)
{
    // std::vector<char> buf;
    char buf[1024] = {0};
    // buf.resize(1024);
    int a = recv(cli, buf, 1023, 0);
    // 1;
    if(a <= 0)
    {
        // perror("Hand_IO error");
        clean_client(cli);
        return;
    }
    buf[a] = '\0';
    auto th = connections.find(cli);
    if(th == connections.end())     return;
    auto connection = th->second;
    // std::cout << buf << std::endl;
    // std::cout << "hand_io" << std::endl;
    connection->process_input(buf, a);
}

void worker::clean_client(int cli)
{
    epoll_ctl(epoll_worker, EPOLL_CTL_DEL, cli, nullptr);
    std::lock_guard<std::mutex> lock(cli_mutex);
    cli_fd.erase(std::remove(cli_fd.begin(), cli_fd.end(), cli), cli_fd.end());
    close(cli);
    return;
}

void worker::join()
{
    if(_thread.joinable())
    _thread.join();
}
