#include "User_chat.h"

void UserChat::print_notify(const chat::Chat& chat_msg) {
    auto proto_result = chat_msg.req().result();
    std::string msg;
    Clear::clearScreen();
    if(proto_result.empty()) {
        std::cout << "\033[33m这里什么也没有...\033[0m" << std::endl;
    }
    for(const auto& [key, value] : proto_result) {
        std::cout << key << "   时间：" << value << std::endl;
        friend_res.push_back(key);
    }
    std::cout << "\033[34m若不处理,输入 9;\033[0m" << std::endl;
    std::cout << "\033[34m若要处理,输入 8;\033[0m" << std::endl;
    // std::cout << "\033[34m按任意键进行退出操作\033[0m" << std::endl;
    // std::getline(std::cin, msg);
    // Clear::clearScreen();    
}

void UserChat::recv_add_friend(const chat::Chat& chat_msg) {
    std::string from_name = chat_msg.friend_req().from_username();
    std::cout << "\033[1;32m" << from_name << "\033[0m"
    << "\033[33m请求添加你为好友\033[0m" << std::endl
    << "\033[33m发送时间：" << chat_msg.friend_req().time() << "\033[0m" << std::endl;
}

void UserChat::print_friends(const chat::Chat& chat_msg) {
    auto proto_frends = chat_msg.friends().friends();
    std::string msg;
    // Clear::clearScreen();
    if(proto_frends.empty()) {
        std::cout << "\033[33m这里什么也没有...\033[0m" << std::endl;
        std::cout << "\033[33m快去添加好友吧...\033[0m" << std::endl;
        return;
    }
    for(const auto& [key, value] : proto_frends) {
        friends.insert(key);
        std::cout << key;
        if(value) {
            std::cout << "\033[1;33m ----在线\033[0m" << std::endl;
        } else {
            std::cout << "\033[1;31m ----离线\033[0m" << std::endl;
        }
    }
}

void UserChat::save_friends(const chat::Chat& chat_msg) {
    auto proto_frends = chat_msg.friends().friends();
    std::string msg;
    for(const auto& [key, value] : proto_frends) {
        // std::cout << key << "ss" << std::endl;
        friends.insert(key);
        // if(value) {
        //     friedns_online[key] = true;
        // }
    }
    auto proto_groups = chat_msg.group_list().groups();
    for(const auto& group : proto_groups) {
        // std::cout << group << "sss" << std::endl;
        user_groups.insert(group);
    }
}

void UserChat::print_delete(const chat::Chat& chat_msg) {
    std::string name = chat_msg.friend_del_res().username();
    if(chat_msg.friend_del_res().result()) {
        std::cout << "\033[34m已删除好友: " << name << "-——-\033[0m" << std::endl;
        friends.erase(name);
        friends_block.erase(name);
        return;
    } else {
        std::cout << "\033[34m❌删除失败\033[0m" << std::endl;
    }
}

void UserChat::printf_delete_req(const chat::Chat& chat_msg) {
    std::string name = chat_msg.friend_del_req().from_name();
    std::string time = chat_msg.friend_del_req().time();
    std::cout << "\033[34m已被 " << name << " 删除-——-\033[0m" << std::endl;
    std::cout << "\033[34m时间为: " << time << "\033[0m" << std::endl;
    friends.erase(name);
    friends_block.erase(name);
}

void UserChat::print_response(const chat::Chat& chat_msg) {

    auto proto_result = chat_msg.response().result();
    std::string marker = "\033[31m已将您解除屏蔽\033[0m";
    std::string marker2 = "\033[31m已将您屏蔽❌\033[0m";
    std::string res;
    if(proto_result.empty()) {
        std::cout << "\033[33m这里什么也没有...\033[0m" << std::endl;
    }
    for(const auto& [key, value] : proto_result) {
        size_t pos = key.find(marker);
        if(pos != std::string::npos) {
            std::string username;
            username = key.substr(0, pos);
            print_unblock(username);
            continue;
        }
        size_t pos2 = key.find(marker2);
        if(pos2 != std::string::npos) {
            std::string username;
            username = key.substr(0, pos2);
            print_block(username);
            continue;
        }
        std::cout << key << std::endl;
        std::cout << "\033[34m时间: " << value << "\033[34m" << std::endl;
    }
}

void UserChat::print_add_friend(const chat::Chat& chat_msg) {
    std::string decides = chat_msg.friend_res().msg();
    std::cout << decides << std::endl;
    if(chat_msg.friend_res().decide()) {
        std::string name = chat_msg.friend_res().from_name();
        std::cout << "\033[36m已成功添加 " << name << " 为好友\033[0m" << std::endl;
        friends.insert(name);
    } else {
        std::string name = chat_msg.friend_res().from_name();
    }
}

void UserChat::print_offline_msg(const chat::Chat& chat_msg) {
    if(chat_msg.off_msg().msg_size() == 0) {
        std::cout << "\033[1;33m 你们还没有聊天，快去聊天吧！\033[0m" << std::endl;
    }
    for(int i = 0; i < chat_msg.off_msg().msg_size(); i++) {
        std::cout << chat_msg.off_msg().msg(i) << std::endl;
    }
    std::cout << "\033[34m输入 9 安全退出\033[0m" << std::endl;
}

void UserChat::print_cancel() {
    std::cout << "\033[1;31m此账号已在别处登陆, 请您重新登陆\033[0m" << std::endl;
    std::cout << "\033[1;31m输入任意键 + <Enter>退出程序...\033[0m" << std::endl;
    _run = false;
    return;
}

void UserChat::print_block(const std::string& chat_msg) {
    std::string name = chat_msg;
    std::cout << name << std::endl;
    if(friends.find(name) != friends.end()) {
        std::cout << name << "\033[1;31m已将您屏蔽❌\033[0m" << std::endl;
        friends_by_block.insert(name);
    } else {
        std::cout << name << "\033[1;31m在解除好友之前将您屏蔽了...\033[0m" << std::endl;
    }
    
}

void UserChat::print_unblock(const std::string& chat_msg) {

    std::string name = chat_msg;
    if(friends.find(name) != friends.end()) {
        std::cout << name << "\033[1;31m已将您解除屏蔽---\033[0m" << std::endl;
        friends_by_block.erase(name);
    } else {
        std::cout << name << "\033[1;31m在解除好友之前将您解除屏蔽了...\033[0m" << std::endl;
    }
}

void UserChat::print_msg(const chat::Chat& chat_msg) {
    std::cout << "\033[1;32m您收到一条新的未读消息,来自 " << chat_msg.chat_fd().from_name() << "\033[0m" << std::endl;
}

void UserChat::print_send_file(const chat::Chat& chat_msg) {
    std::string name = chat_msg.send_file().name();
    std::string time = chat_msg.send_file().time();
    std::cout << "\033[1;34m 您的好友 " << "\033[1;34m" << name << "\033[0m" << " 给您发来一份文件\033[0m" << std::endl;
    std::cout << "\033[1;33m 时间是：" << time << "\033[0m" << std::endl;
}

void UserChat::print_look_file(const chat::Chat& chat_msg) {
    std::unordered_map<std::string, std::vector<std::string>> result;
    if(chat_msg.file_req().result().empty()) {
        std::cout << "\033[1;33m没有好友向您发送文件---\033[0m" << std::endl;
        return;
    }
    for(const auto& [friend_name, friend_files] : chat_msg.file_req().result()) {
        for(const auto& file : friend_files.files()) {
            std::cout << friend_name << "   " <<  "\033[36m" << file << "\033[0m" << std::endl;
            this->friends_files[friend_name].push_back(file);
        }
    }
    std::cout << "\033[1;34m若要处理,请输入 13;\033[0m" << std::endl;
    std::cout << "\033[1;34m若不处理,请输入 9 进行安全退出;\033[0m" << std::endl;
}

void UserChat::print_online_msg(const chat::Chat& chat_msg) {
    // std::cout << "online" << std::endl;
    for(int i = 0; i < chat_msg.off_msg().msg_size(); i++) {
        std::cout << chat_msg.off_msg().msg(i) << std::endl;
    }
    if(chat_msg.off_msg().msg_size() != 0) {
        std::cout << "\033[1;34m=======以上是历史消息=======\033[0m" << std::endl;
    }
}
