#include "User_auth_s.h"

UserAuth_s::UserAuth_s(int port):
_port(port),
listen_sockfd(-1),
_epoll(-1),
flag(false),
_ip(IP),
redis_("tcp://127.0.0.1:6379")
{}

UserAuth_s::~UserAuth_s(){
    if(listen_sockfd >= 0)
    close(listen_sockfd);
    google::protobuf::ShutdownProtobufLibrary();
}

int UserAuth_s::make_nonblocking(int sockfd){
    int flags = fcntl(sockfd, F_GETFL);
    if(flags == -1)
    return -1;
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void UserAuth_s::init(){
    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(make_nonblocking(listen_sockfd) == -1){
        perror("make_nonblocking_listen ERROR!!!");
        exit(1);
    }
    sockaddr_in sock;
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = INADDR_ANY;
    sock.sin_port = htons(_port);
    std::cout << _port << std::endl;
    std::cout << _ip << std::endl;
    // 即使端口处于 TIME_WAIT 状态，我也想重用它～
    int opt = 1;
    setsockopt(listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(listen_sockfd, (sockaddr*)&sock, sizeof(sock)) == -1){
        perror("bind_listen ERROR!!!");
        exit(1);
    }
    if(listen(listen_sockfd, NUM) == -1){
        perror("set_listen ERROR!!!");
        exit(1);
    }
    
    run();
}

void UserAuth_s::run(){
    _epoll = epoll_create1(0);
    struct epoll_event ev{}, events[MAX_EVENT];
    ev.data.fd = listen_sockfd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(_epoll, EPOLL_CTL_ADD, listen_sockfd, &ev);
    while(true){
        int n = epoll_wait(_epoll, events, MAX_EVENT, -1);
        if(n < 0){
            if(errno == EINTR) continue;
        }
        for(int i = 0; i < n; i++){
            int fd = events[i].data.fd;
            if(fd == listen_sockfd){
                accept_cli();
            }
            else{
                onMessage(fd);
            }
        }
    }
}

void UserAuth_s::accept_cli(){
    while(1){
        sockaddr_in sock_cli;
        socklen_t len = sizeof(sock_cli);
        int cli_next = accept(listen_sockfd, (sockaddr*)&sock_cli, &len);
        if(cli_next < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            perror("accept_listen ERROE!!!");
            break;
        }
        make_nonblocking(cli_next);
        // _cli.push(cli_next);
        struct epoll_event ev_cli{};
        ev_cli.data.fd = cli_next;
        ev_cli.events = EPOLLIN | EPOLLET;
        epoll_ctl(_epoll, EPOLL_CTL_ADD, cli_next, &ev_cli);
        std::cout << "Welcome: " << cli_next << std::endl;

        handle_c[cli_next] = clientHandle();
    }
}

void UserAuth_s::onMessage(int cli){
    char buf[4096];
    ssize_t a = read(cli, buf, sizeof(buf));
    if(a == 0){
        std::cout << "bey bey~" << std::endl;
        close(cli);
        epoll_ctl(_epoll, EPOLL_CTL_DEL, cli, nullptr);
        handle_c.erase(cli);
        return;
    }
    else if(a > 0){
        handle_c[cli].handle_buffer.append(buf, a);

        auth::Auth auth_msg;
        handleProcess(cli, auth_msg);
        switch(auth_msg.action()){
            case auth::actions::UNKNOWN:
                Process(cli, auth_msg.msg());
                break;
            case auth::actions::REGISTER:
                Register_R(cli, auth_msg);
                break;
            case auth::actions::LOGIN:
                LogIn(cli, auth_msg);
                break;
            case auth::actions::RVERIFY:
                R_check(cli, auth_msg);
                break;
            case auth::actions::LVERIFY:
                L_check(cli, auth_msg);
                break;
        }
    }
    memset(buf, 0, sizeof(buf));
}

// 验证码识别
void UserAuth_s::R_check(int cli, const auth::Auth& auth_msg){
    std::string buf = auth_msg.r_ver().username();
    std::cout << auth_msg.r_ver().email_msg() << std::endl;

    auth::Auth auth_res;
    auth_res.set_action(auth::actions::RVERIFY);
    auth::Register_Ver* ver = auth_res.mutable_r_ver();
    std::string time = GetNowTime();
    ver->set_time(time);

    if(auth_msg.r_ver().email_msg() == redis_.userFieldExists(buf)){
        std::cout << "Registration Successful." << std::endl;
        redis_.saveUser(auth_msg.r_ver().username(), user_info);
        ver->set_decide(true);
    } else {
        std::cout << "Registration Failed." << std::endl;
        // redis_.deleteHashKey(auth_msg.r_ver().username());
        ver->set_decide(false);
    }

    if(!auth_res.SerializeToString(&buf)){
        std::cerr << "Serialize error" << std::endl;
        std::cout << "Registration Failed." << std::endl;
    } else {
        Send(cli, buf);
    }
}

void UserAuth_s::L_check(int cli, const auth::Auth& auth_msg){
    std::string buf = auth_msg.l_ver().username();

    auth::Auth auth_res;
    auth_res.set_action(auth::actions::LVERIFY);
    auth::Login_Ver* ver = auth_res.mutable_l_ver();
    std::string time = GetNowTime();
    ver->set_time(time);

    if(redis_.userFieldExists(buf) == auth_msg.l_ver().email_msg()){
        std::cout << "Number " << cli << " Login Successful." << std::endl;
        redis_.addUserToOnlineLists(auth_msg.l_ver().username());
        ver->set_decide(true);
    } else {
        std::cout << "Login Failed." << std::endl;
        ver->set_decide(false);
    }

    std::string send_buf;
    if(!auth_res.SerializeToString(&send_buf)){
        std::cerr << "Serialize error" << std::endl;
        std::cout << "Login Failed." << std::endl;
    } else {
        std::cout << "buf: " << send_buf;
        Send(cli, send_buf);
    }
}


// 解决粘包问题
void UserAuth_s::handleProcess(int cli, auth::Auth& auth_msg){
    while(handle_c[cli].handle_buffer.size() >= 4){
        uint32_t len = 0;
        memcpy(&len, handle_c[cli].handle_buffer.data(), 4);
        len = ntohl(len);

        if(handle_c[cli].handle_buffer.size() < len + 4) break;

        std::string msg = handle_c[cli].handle_buffer.substr(4, len);        
        if(!auth_msg.ParseFromString(msg)){
            std::cerr << "Parse error!" << std::endl;
            break;
        }
        handle_c[cli].handle_buffer.erase(0, 4 + len);
    }
}

void UserAuth_s::Process(int cli, const std::string& buf){
    if(strncmp(buf.data(), "register", 8) == 0){
        Send(cli, "Please enter your user_message:");
    } else if(strncmp(buf.data(), "login", 5) == 0){
        Send(cli, "Please enter your message:");
    }
}

// 发送验证码
void UserAuth_s::Send_verify(const std::string& username, const std::string& email){
    MailSender sender("smtps://smtp.qq.com:465",
                      "jiaguoyangyang@qq.com",
                      "aauzmklxescgdbjb");
    std::string from = "jiaguoyangyang@qq.com";
    std::cout << email << std::endl;
    std::vector<std::string> to = {email};
    std::string subject = "Your Verification Code";
    std::string code = generate_Ver();
    std::string content = "Your verification code is " + code;
    if(sender.sendMail(from, to, subject, content)){
    std::cout << "send_verification successful" << std::endl;
    } else {
        std::cout << "send_verification Failed" << std::endl;
        // return;
    }
    redis_.setKeyExpire(username, code, 300);
}

// 生成验证码
std::string UserAuth_s::generate_Ver(){
    std::string code;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);
    for(int i = 0; i < 6; i++){
        code += std::to_string(dis(gen));
    }
    return code;
}

// 注册反序列并处理
void UserAuth_s::Register_R(int cli, const auth::Auth& auth_msg){
    
    const auth::RegisterRequest& req = auth_msg.req();
    user_info = {
        {"password", req.password()},
        {"email", req.email()},
        {"phone", req.phone()}
    };

    bool a = true;
    std::string msg = "Success!";
    if(!redis_.saveUser(req.username(), user_info)){
        a = false;
        msg = "Fail!";
        std::cerr << "User registration failed" << std::endl;
    }
    redis_.deleteHashKey(req.username());
    std::string buf = Ser_R(a, msg);
    Send(cli, buf);

    buf = req.username();
    if(a) Send_verify(buf, req.email());

}

std::string UserAuth_s::Ser_R(bool a, const std::string& msg){
    auth::Auth auth_msg;
    auth_msg.set_action(auth::actions::REGISTER);
    auth::RegisterResponse* sp = auth_msg.mutable_res();
    sp->set_decide(a);
    sp->set_msg(msg);
    // sp->set_time(GetNowTime());

    std::string buf;
    auth_msg.SerializeToString(&buf);

    return buf;
}

void UserAuth_s::Send(int cli, const std::string& buf){
    ssize_t a = ::send(cli, buf.data(), buf.size(), 0);
}

std::string UserAuth_s::GetNowTime(){
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&now_time);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
    return buf;
}

// 登陆反序列并处理
void UserAuth_s::LogIn(int cli, const auth::Auth& auth_msg){

    const auth::LogInRequest& loq = auth_msg.loq();
    
    std::string ser_l;
    // std::string u_name = "user:" + loq.username();
    if(auth_msg.loq().select() == auth::Select::PASSWORD){
        int32_t b = 0;
        if(redis_.userFieldHexists(loq.username(), "password")){
            auto u_pass = redis_.getUserField(loq.username(), "password");
            if(u_pass == loq.password()){
                std::cout << "Number " << cli << " login successful." << std::endl;\
                redis_.addUserToOnlineLists(loq.username());
                b = 1;
            } else {
                ser_l = "\033[31m密码错误\033[0m";
                std::cout << ser_l << std::endl;
                b = 2;
            }
        } else {
            ser_l = "\033[31m用户不存在,请注册\033[0m";
            std::cout << ser_l << std::endl;
            b = 0;
        }
        // std::cout << "b:" << b << std::endl;
        ser_l = Ser_L(b);
        // std::cout << ser_l << std::endl;
        Send(cli, ser_l);
    } else {
        Send_verify(loq.username(), auth_msg.loq().email());
    }
}

std::string UserAuth_s::Ser_L(int32_t b){
        // std::cout << "b:" << b << std::endl;
    auth::Auth auth_msg;
    auth_msg.set_action(auth::actions::LOGIN);
    auth::LogInResponse* los = auth_msg.mutable_los();
    los->set_decide(b);
    std::string s;
    auth_msg.SerializeToString(&s);
    auth_msg.ParseFromString(s);
    // std::cout << "decide:" << auth_msg.los().decide() << std::endl;
    auth_msg.SerializeToString(&s);
    return s;
}







