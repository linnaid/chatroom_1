#include "masterreactor.h"


master::master(int port):
    _port(port),
    listen_sockfd(-1),
    epoll_master(-1),
    _worker(0),
    _time(-1),
    cli_fd(-1)
    {}

master::~master()
    {
        if(epoll_master >= 0)
        close(epoll_master);
        if(listen_sockfd >= 0)
        close(listen_sockfd);
    }

void master::init()
{
    // std::cout << "sss" << std::endl;
    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 1
    if(listen_sockfd < 0)
    {
        perror("listen_sockfd created error");
        exit(1);
    }
    // 1
    if(make_nonblocking(listen_sockfd) == -1)
    {
        perror("make_nonblocking error");
        exit(1);
    }
    sockaddr_in sock;
    sock.sin_addr.s_addr = INADDR_ANY;
    sock.sin_family = AF_INET;
    sock.sin_port = htons(_port);

    // 即使端口处于 TIME_WAIT 状态，我也想重用它~
    int opt = 1;
    setsockopt(listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 1
    if(bind(listen_sockfd, (sockaddr*)&sock, sizeof(sock)) == -1)
    {
        perror("bind_listen error");
        exit(1);
    }
    if(listen(listen_sockfd, NUM) == -1)
    {
        perror("listen_listen error");
        exit(1);
    }
    for(int i = 0; i < NUM; i++)
    {
        workers.emplace_back(std::make_unique<worker>());
        workers.back()->start();
    }

    run();
}

int master::make_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL);
    if(flags == -1)
    return -1;
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void master::run()
{
    // std::cout << "qqq" << std::endl;
    epoll_master = epoll_create1(0);
    struct epoll_event ev{}, events[MAX_EVENT];
    ev.data.fd = listen_sockfd;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_master, EPOLL_CTL_ADD, listen_sockfd, &ev);
    while(true)
    {
        int n = epoll_wait(epoll_master, events, MAX_EVENT, -1);
        if(n < 0)
        {
            if(errno == EINTR) continue;
        }
        for(int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            if(fd == listen_sockfd)
            {
                accept_client();
                // std::cout << "link" << std::endl;
            }
            // else{
            //     workers[]
            // }
        }
    }
}

void master::accept_client()
{
    while(true)
    {
        sockaddr_in cli;
        socklen_t len = sizeof(cli);
        cli_fd = accept(listen_sockfd, (sockaddr*)&cli, &len);
        if(cli_fd < 0)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            perror("accept Error");
            break;
        }
        // std::cout << "ssss:" << cli_fd << std::endl;

        if(make_nonblocking(cli_fd) < 0)
        perror("cli_nonblocking error");
        distribute_worker(&cli_fd);
    } 
}

void master::distribute_worker(int *cli_fd)
{
    auto& work = workers[_worker++ % workers.size()];
    // std::cout << _worker % workers.size() << "_worker" << std::endl;
    work->make_queue(cli_fd);
}

