#pragma once

#include "include.h"
#include "user_auth.pb.h"
#include "user_page.h"
#include "Clear.hpp"

extern std::string _username;

class auth_Page;

class UserAuth_c{
public:
    UserAuth_c(int argc, char** argv);
    ~UserAuth_c();
    void init();
private:
    void run();
    std::string Register_u();
    std::string LogIn();
    bool register_Ver();
    bool login_Ver();
    void get_verify(auth::Auth& auth_msg, int type);
    void Parse_u(const std::string& buf);
    void R_check(auth::Auth& auth_msg);
    void L_check(auth::Auth& auth_msg);

    
    int _sockfd;
    int _port;
    bool log_i;
    int a;
    std::string _user;
    std::string _ip;
    sockaddr_in _cli;
};