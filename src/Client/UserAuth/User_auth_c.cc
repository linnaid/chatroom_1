#include "User_auth_c.h"

std::string _username;

UserAuth_c::UserAuth_c(int argc, char** argv):
_port(PORT1),
_sockfd(-1),
a(1),
log_i(false)
{
    if(argc == 3) {
        IP = argv[1];
        _ip = IP;
    } else{
        _ip = IP;
    }
}

UserAuth_c::~UserAuth_c(){
    if(_sockfd >= 0)
    close(_sockfd);
    google::protobuf::ShutdownProtobufLibrary();
}

void UserAuth_c::init(){
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(_sockfd == -1){
        perror("sockfd-make ERROR!!!");
        exit(1);
    }
    // std::cout << _port << std::endl;
    // std::cout << _ip << std::endl;
    _cli.sin_family = AF_INET;
    _cli.sin_port = htons(_port);
    ::inet_pton(AF_INET, _ip.c_str(), &_cli.sin_addr.s_addr);
    if(connect(_sockfd, (sockaddr*)&_cli, sizeof(_cli)) < 0) {
        std::cerr << "connect error" << std::strerror(errno) << std::endl;
        return;
    }
    run();
}

void UserAuth_c::run(){
    // char buf[1024];
    std::string buf;
    int decide;
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    while(true){
        auth_Page::Start();
        std::cin >> decide;
        // std::cout << decide << std::endl;
        if(decide == 1) {
            buf = "login";
            // std::cout << buf << std::endl;
        } else if(decide == 2) {
            buf = "register";
        } else if (std::cin.fail()){
            Clear::clearScreen();
            std::cout << "\033[1;31m输入错误,请重新输入:\033[0m" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        } else if(decide == 0) {
            buf = "QUIT";
        } else {
            Clear::clearScreen();
            std::cout << "\033[1;31m输入错误,请重新输入:\033[0m" << std::endl;
            // std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        auth::Auth auth_msg;
        auth_msg.set_msg(buf);
        std::string buffer;
        auth_msg.SerializeToString(&buffer);
        uint32_t len = htonl(buffer.size());
        // std::string packet_msg(reinterpret_cast<char*>(&len), 4);
        std::string packet_msg;
        packet_msg.resize(4);
        memcpy(&packet_msg[0], &len, 4);
        packet_msg += buffer;
        ssize_t a = send(_sockfd, packet_msg.data(), packet_msg.size(), 0);
        if(a < 0){
            perror("Send ERROR");
        } else if(a == 0) {
            std::cout << "服务器关闭连接" << std::endl;
            return;
        }
        if(strcmp(buf.data(), "QUIT") == 0){
            break;
        }
        char buff[1024];
        ssize_t b = recv(_sockfd, buff, sizeof(buff), 0);
        // std::cout << buff << std::endl;
        std::string ser_user;
        if(strncmp(buff, "Please enter your user_message:", strlen("Please enter your user_message:")) == 0){
            ser_user = Register_u();
            len = htonl(ser_user.size());
            // std::string packet_reg(reinterpret_cast<char*>(len), 4);
            std::string packet_reg;
            packet_reg.resize(4);
            memcpy(&packet_reg[0], &len, 4);
            packet_reg += ser_user;
            send(_sockfd, packet_reg.data(), packet_reg.size(), 0);
            if(!register_Ver()) continue;
        }else if(strncmp(buff, "Please enter your message:", strlen("Please enter your message:")) == 0){
            ser_user = LogIn();
            len = htonl(ser_user.size());
            std::string packet_log;
            packet_log.resize(4);
            memcpy(&packet_log[0], &len, 4);
            packet_log += ser_user;
            send(_sockfd, packet_log.data(), packet_log.size(), 0);
            if(!login_Ver()) continue;
        }
        if(log_i) break;
        memset(buff, 0, sizeof(buff));
        ssize_t c = recv(_sockfd, buff, sizeof(buff), 0);
        Parse_u(buff);
        memset(buff, 0, sizeof(buff));
        if(log_i) break;
    }
}

bool UserAuth_c::register_Ver(){
    auth::Auth auth_recv;
    char buff[1024];
    ssize_t a = recv(_sockfd, buff, sizeof(buff), 0);
    std::string msg(buff, a);
    if(!auth_recv.ParseFromString(msg)){
        std::cerr << "Parse error" << std::endl;
    }
    if(!auth_recv.res().decide()){
        std::cout << "Register failedssss" << std::endl;
        _user = "";
        return false;;
    }
    auth::Auth auth_msg;
    auth_msg.set_action(auth::actions::RVERIFY);
    get_verify(auth_msg, 1);
    return true;
}

bool UserAuth_c::login_Ver(){
    if(a == 1){
        auth::Auth auth_recv;
        char buff[1024];
        ssize_t a = recv(_sockfd, buff, sizeof(buff), 0);
        std::string msg(buff, a);
        // std::cout << buff << std::endl;
        if(!auth_recv.ParseFromString(msg)){
            // std::cerr << "Parse error." << std::endl;
            return false;
        }
        std::cout << auth_recv.los().decide() << std::endl;
        if(auth_recv.los().decide() == 0){
            std::cout << "\033[31m用户不存在,请注册\033[0m" << std::endl;
            _username = "";
            return false;
        } else if (auth_recv.los().decide() == 2) {
            std::cout << "\033[31m密码错误\033[0m" << std::endl;
            _username = "";
        } else if(auth_recv.los().decide() == 1) {
            std::cout << "\033[34m登陆成功\033[0m" << std::endl;
            log_i = true;
            return true;
        }
        return false;
    }
    auth::Auth auth_msg;
    auth_msg.set_action(auth::actions::LVERIFY);
    get_verify(auth_msg, 2);
    return true;
}

void UserAuth_c::get_verify(auth::Auth& auth_msg, int type){
    uint32_t len;
    std::string input;
    // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "\033[34m请输入验证码:\033[0m";
    getline(std::cin, input);
    if(type == 1) {
        auth::Register_Ver* ver = auth_msg.mutable_r_ver();
        ver->set_email_msg(input);
        ver->set_username(_user);
    } else if(type == 2) {
        auth::Login_Ver* ver = auth_msg.mutable_l_ver(); 
        ver->set_email_msg(input);
        ver->set_username(_username);
    }
    if(!auth_msg.SerializeToString(&input)){
        std::cerr << "Serialize error" << std::endl;
    }
    len = htonl(input.size());
    std::string msg;
    msg.resize(4);
    memcpy(&msg[0], &len, 4);
    msg += input;
    send(_sockfd, msg.data(), msg.size(), 0);
}

void UserAuth_c::Parse_u(const std::string& buf){
    auth::Auth auth_msg;
    // std::cout << buf << std::endl;
    if(!auth_msg.ParseFromString(buf)){
        std::cerr << "Parse error111" << std::endl;
    }
    switch(auth_msg.action()){
        case auth::actions::RVERIFY:
            R_check(auth_msg);
            break;
        case auth::actions::LVERIFY:
            L_check(auth_msg);
            break;
    }
}

void UserAuth_c::L_check(auth::Auth& auth_msg){
    std::cout << "sss"<< std::endl;
    if(auth_msg.l_ver().decide()){
        std::cout << "\033[34m登陆成功\033[0m" << std::endl;
        log_i = true;
    } else{
        _username = "";
        std::cout << "\033[31m登陆失败\033[0m" << std::endl;
    }
}

void UserAuth_c::R_check(auth::Auth& auth_msg){
    if(auth_msg.r_ver().decide()){
        std::cout << "\033[36m注册成功！\033[0m" << std::endl;
        // std::cout << "your_id:" << sp.user_id() << std::endl;
        std::cout << "\033[34m你的注册时间为:\033[0m" << std::endl;
        std::cout << auth_msg.r_ver().time() << std::endl;
    } else{
        std::cout << "\033[31m啊哦，注册失败" << std::endl <<"记住，用户名不能重复！邮箱要有效！验证码要输对！\033[0m" << std::endl;
    }
}

// 实现注册的序列化
std::string UserAuth_c::Register_u(){
    
    auth::Auth auth_msg;
    auth_msg.set_action(auth::actions::REGISTER);
    auth::RegisterRequest* req = auth_msg.mutable_req();
    std::string input;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "\033[34m请输入你的用户名:\033[0m";
    getline(std::cin, input);
    if(input == "quit" || input == "QUIT") {
        std::cout << "\033[31m用户名不能为<quit>/<QUIT>!\033[0m" << std::endl;
        std::cout << "\033[34m请重新输入用户名:\033[0m" << std::endl;
        while(true) {
            getline(std::cin, input);
            if(input == "quit" || input == "QUIT") {
                std::cout << "\033[31m用户名不能为<quit>/<QUIT>!\033[0m" << std::endl;
                std::cout << "\033[34m请重新输入用户名:\033[0m" << std::endl;
            } else {
                break;
            }
        }

    }
    req->set_username(input);
    _user = input;
    std::cout << "\033[34m请输入你的密码:\033[0m";
    getline(std::cin, input);
    req->set_password(input);
    std::cout << "\033[34m请输入你的邮箱:\033[0m";
    getline(std::cin, input);
     req->set_email(input);
    std::cout << "\033[34m请输入你的移动电话:\033[0m";
    getline(std::cin, input);
    req->set_phone(input);

    std::string req_user;
    if(!auth_msg.SerializeToString(&req_user)){
        std::cerr << "Serialize ERROR" << std::endl;;
        return "";
    }
    return req_user;
}

// 实现登陆的序列化
std::string UserAuth_c::LogIn(){

    auth::Auth auth_msg;
    auth_msg.set_action(auth::actions::LOGIN);
    auth::LogInRequest* loq = auth_msg.mutable_loq();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string input;
    
    std::cout << "\033[34m请输入你的用户名:\033[0m";
    getline(std::cin, input);
    loq->set_username(input);
    _username = input;
    std::cout << "\033[34m密码登陆(1)    验证码登陆(2)\033[0m" << std::endl;
    std::cout << "\033[34m请作出你的选择:\033[0m";
    while(true) {
        std::cin >> a;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if(a == 1) {
            std::cout << "\033[34m请输入你的密码:\033[0m";
            getline(std::cin, input);
            loq->set_password(input);
            loq->set_select(auth::Select::PASSWORD);
            break;
        } else if(a == 2) {
            std::cout << "\033[34m请输入你的邮箱:\033[0m";
            getline(std::cin, input);
            loq->set_email(input);
            loq->set_select(auth::Select::EMAIL);
            break;
        } else {
            Clear::clearScreen();
            std::cout << "\033[1;31m输入错误,请重新输入:\033[0m" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
    }
    std::string user_loq;
    if(!auth_msg.SerializeToString(&user_loq)){
        std::cerr << "Seria_LOQ error" << std::endl;
        return nullptr;
    }
    return user_loq;
}




