#include "Heart.h"

std::atomic<bool> running{true};

Heart::Heart()
: _ip(IP),
_sockfd(-1),
_port(PORT2)
{}

Heart::~Heart() {
    if(_sockfd >= 0) {
        close(_sockfd);
    }
}

void Heart::init() {

    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(_sockfd == -1) {
        std::cerr << "Socket Error in User_chat:" << std::strerror(errno) << std::endl;
    }
    _fd.sin_family = AF_INET;
    _fd.sin_port = htons(_port);
    ::inet_pton(AF_INET, _ip.c_str(), &_fd.sin_addr.s_addr);
    if(connect(_sockfd, (sockaddr*)&_fd, sizeof(_fd)) == -1) {
        std::cout << "hello" << std::endl;
    }

    send_heart();
}

void Heart::send_heart() {
    using namespace std::chrono_literals;

    chat::Chat chat_heart;
    chat_heart.set_action(chat::Actions::HEART);
    chat_heart.set_heart("ping");
    std::string msg;
    chat_heart.SerializeToString(&msg);
    msg = Protocol::pack(msg);
    while(running) {
        std::cout << "正在跳动" << std::endl;
        ssize_t a = send(_sockfd, msg.c_str(), msg.size(), 0);
        if(a < 0) {
            std::cerr << "Send Error in User_chat:" << std::strerror(errno) << std::endl;           
        }
        std::this_thread::sleep_for(30s);
    }
}