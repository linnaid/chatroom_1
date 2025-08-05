#pragma once

#include "include.h"
#include "user_auth.pb.h"
#include "RedisClient.h"
#include "MailSender.h"
#include "user_page.h"


struct clientHandle{
    std::string handle_buffer;
};

class UserAuth_s{
public:
    UserAuth_s(int port);
    ~UserAuth_s();
    void init();
    void run();
private:
    int make_nonblocking(int sockfd);
    void onMessage(int cli);
    void accept_cli();
    void Process(int cli, const std::string& buf);
    void Send(int cli, const std::string& buf);
    std::string Ser_R(bool a, const std::string& msg);
    std::string Ser_L(int32_t b);
    void Register_R(int cli, const auth::Auth& rec);
    void LogIn(int cli, const auth::Auth& rec);
    std::string GetNowTime();
    void handleProcess(int cli, auth::Auth& auth_msg);
    std::string generate_Ver();
    void Send_verify(const std::string& username, const std::string& email);
    void R_check(int cli, const auth::Auth& auth_msg);
    void L_check(int cli, const auth::Auth& auth_msg);

    int _port;
    std::string _ip;
    // std::queue<int> _cli;
    std::unordered_map<int, clientHandle> handle_c; 
    RedisClient redis_;
    std::unordered_map<std::string, std::string> user_info;
    int listen_sockfd;
    int _epoll;
    bool flag;
    // int u_id = 1;
};
