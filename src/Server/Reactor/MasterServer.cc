#include "MasterServer.h"

std::unordered_map<int, int> Worker_fd;

Master::Master()
    : _redis("tcp://127.0.0.1:6379"),
      _port(PORT2),
      _worker(0),
      epoll_master(-1),
      listen_sockfd(-1)
{
}

Master::~Master()
{
    if (epoll_master >= 0)
    {
        close(epoll_master);
    }
}

void Master::init()
{
    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (make_nonblocking(listen_sockfd) == -1) {
        std::cerr << "Make_nonblocking error in Master" << std::strerror(errno) << std::endl;
    }

    sockaddr_in sock;
    sock.sin_family = AF_INET;
    sock.sin_port = htons(_port);
    sock.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(listen_sockfd, (sockaddr *)&sock, sizeof(sock)) == -1) {
        std::cerr << "Bind error in Master" << std::endl;
    }

    if (listen(listen_sockfd, NUM) == -1) {
        std::cerr << "Bind error in Master" << std::endl;
    }

    for (int i = 0; i < WORKER; i++)
    {
        workers.emplace_back(std::make_shared<Worker>(i));
        MessageCenter::instance().register_worker(i, workers.back());
        workers.back()->start();
    }
    
    run();
}

void Master::run()
{
    epoll_master = epoll_create1(0);
    struct epoll_event ev, events[NUM];
    ev.data.fd = listen_sockfd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epoll_master, EPOLL_CTL_ADD, listen_sockfd, &ev) == -1)
    {
        std::cerr << "epoll_ctl failed: " << std::strerror(errno) << std::endl;
    }
    while (true)
    {
        int n = epoll_wait(epoll_master, events, NUM, TIME);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
        }
        std::cout << "hello3" << std::endl;
        for (int i = 0; i < n; i++)
        {
            std::cout << "hello2" << std::endl;
            int fd = events[i].data.fd;
            if (fd == listen_sockfd)
                accept_client();
        }
    }
}

int Master::make_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        return flags;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Master::accept_client()
{

    while (true)
    {
        sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int cli_fd = accept(listen_sockfd, (sockaddr *)&cli, &len);
        std::cout << "\033[34mWelcome: " << cli_fd << "\033[0m" << std::endl;
        if (cli_fd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            std::cerr << "accept error in Master" << std::endl;
            break;
        }
        std::cout << "hello1" << std::endl;
        distribute_worker(cli_fd);
    }
}

void Master::distribute_worker(int cli_fd)
{
    std::cout << cli_fd << "已进入";
    int num = _worker++ % WORKER;
    Worker_fd[cli_fd] = num;
    auto work = workers[num];
    work->make_queue(cli_fd);
}