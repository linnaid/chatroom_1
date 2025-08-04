#include "User_chat.h"

void UserChat::delete_add_friend(const std::string& name) {
    chat::Chat chat_msg;
    chat_msg.set_action(chat::Actions::UNUSERNTF);
    chat::UserRequest* res = chat_msg.mutable_req();
    res->set_username(name);

    std::string msg;
    chat_msg.SerializeToString(&msg);

    Send(msg);
}

void UserChat::decide_add_friend(const std::string& to_name, int select) {
    chat::Chat chat_msg;
    chat_msg.set_action(chat::Actions::ADDFDRES);
    chat::AddFriendResponse* fd_res = chat_msg.mutable_friend_res();
    fd_res->set_to_name(to_name);
    if(select == 1) {
        fd_res->set_decide(true);
    } else if(select == 2) {
        fd_res->set_decide(false);
    }
    
    fd_res->set_from_name(_username);
    std::string msg;
    chat_msg.SerializeToString(&msg);
    Send(msg);
}

void UserChat::joinThread(std::thread& t) {
    if(t.joinable()) {
        t.join();
    }
}

bool UserChat::Send(const std::string& buf) {
    std::string msg = Protocol::pack(buf);
    ssize_t a = send(_sockfd, msg.c_str(), msg.size(), 0);
    if(a < 0) {
        std::cerr << "Send Error in User_chat:" << std::strerror(errno) << std::endl;
        return false;
    }
    return true;
}

std::string UserChat::trim(const std::string& str) {
    ssize_t start = str.find_first_not_of(" \t");
    ssize_t end = str.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

std::pair<std::string, std::string> UserChat::split_dash(const std::string& str) {
    auto pos = str.find("---");
    if(pos == std::string::npos) return {trim(str), ""};

    return {trim(str.substr(0, pos)), trim(str.substr(pos + 3))};
}

