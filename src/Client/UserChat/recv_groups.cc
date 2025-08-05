#include "User_chat.h"


void UserChat::recv_add_group(const chat::Chat& chat_msg) {
    std::string msg = chat_msg.add_group().msg();
    std::cout << msg << std::endl;
    if(!chat_msg.add_group().success()) {
        return;
    }
    std::string u_name = chat_msg.add_group().username();
    std::string g_name = chat_msg.add_group().group_name();
    std::string name = g_name + "   ---" + u_name;
    user_groups.insert(name);
}

void UserChat::print_join_group(const chat::Chat& chat_msg) {
    std::string u_name = chat_msg.join_res().u_name();
    std::string g_name = chat_msg.join_res().g_name();
    std::string name = g_name + "   ---" + u_name;
    if(chat_msg.join_res().decide()) {
        std::cout << "\033[1;32m您已加入群聊" << name << "\033[0m" << std::endl;
        user_groups.insert(name);
    } else {
        std::string msg = chat_msg.join_res().msg();
        std::cout << "\033[1;31m❌加入群聊失败\033[0m" << std::endl;
        std::cout << msg << std::endl;
    }
}

void UserChat::decide_join_group(const chat::Chat& chat_msg) {
    std::cout << "\033[33m您收到一条加群申请,可前往申请界面查看并处理\033[0m" << std::endl;    
}

void UserChat::print_group_list(const chat::Chat& chat_msg) {
    auto groups = chat_msg.group_list().groups();
    if(groups.empty()) {
        std::cout << "\033[35m您还没有加入群聊,快去添加吧!\033[0m" << std::endl;
        return;
    }
    for(const auto& group : groups) {
        std::cout << "\033[1;36m" << group << "\033[0m" << std::endl;
        user_groups.insert(group);
    }
}

void UserChat::print_group_notify(const chat::Chat& chat_msg) {
    auto groups = chat_msg.group_notify().response();
    if(groups.empty()) {
        std::cout << "\033[34m还没有任何加群申请...\033[0m" << std::endl;
        std::cout << "\033[34m已为您安全退出...\033[0m" << std::endl;
        return;
    }
    chat::Chat chat_group;
    for(const auto& [user, value] : groups) {
        
        std::string msg = value;
        if(!Protocol::unpack(msg, chat_group)) {
            break;
        }
        std::string username = chat_group.join_group().username();
        std::string u_name = chat_group.join_group().u_name();
        std::string g_name = chat_group.join_group().g_name();
        std::string name = g_name + "   ---" + u_name;
        std::string time = chat_group.join_group().time();
        std::cout << "\033[1;32m" << user 
        << "\033[34m" << "   " << name << "\033[0m" << std::endl;
        std::cout << "\033[34m时间: " << time << "\033[0m" << std::endl;

    }
    std::cout << "\033[34m如果要处理申请,输入 6;\033[0m" << std::endl;
    std::cout << "\033[34m如果不处理申请,输入 13 安全退出;\033[0m" << std::endl;
}

void UserChat::recv_choice_groups(const chat::Chat& chat_msg) {
    std::string u_name = chat_msg.group_choice().u_name();
    std::string g_name = chat_msg.group_choice().g_name();
   
    _name = g_name + "   ---" + u_name;
    //  std::cout << _name << std::endl;
    _statue = chat_msg.group_choice().statue();
    //  std::cout << _statue << std::endl;
    user_name = u_name;
    group_name = g_name;
    std::cout << "\033[1;32m您已进入群聊!\033[0m" << std::endl;
}

void UserChat::print_group_members(const chat::Chat& chat_msg) {
    const auto& members = chat_msg.group_members().members();
    if(members.empty()) {
        std::cout << "\033[34m群聊里没有成员\033[0m" << std::endl;
    }
    for(const auto& [user, statue] : members) {
        std::cout << user << "---\033[1;37m" << statue << "\033[0m" << std::endl;
    }
    std::cout << "\033[34m输入 13 安全退出...\033[0m" << std::endl;
}

void UserChat::print_chat_group_list(const chat::Chat& chat_msg) {
    const auto& messages = chat_msg.chat_list().msg();
    for(const auto& message : messages) {
        std::cout << message << std::endl;
    } 
    std::cout << "\033[34m输入 13 安全退出...\033[0m" << std::endl;
}

void UserChat::print_group_msg(const chat::Chat& chat_msg) {
    std::string u_name = chat_msg.group_chat().u_name();
    std::string g_name = chat_msg.group_chat().g_name();
   
    std::string name = g_name + "   ---" + u_name;
    std::cout << "\033[1;33m您收到一条新的未读群消息,来自 \033[1;32m" << chat_msg.group_chat().username() << "\033[0m" << std::endl;
    std::cout << "\033[1;33m属于群: " << name << "\033[0m" << std::endl;
}

void UserChat::print_leave_group(const chat::Chat& chat_msg) {
    if(chat_msg.leave_group().decide()) {
        std::cout << "\033[32m已成功退出群聊---\033[0m" << std::endl;
        user_groups.erase(_name);
    } else {
        std::cout << "\033[31m退出群聊失败\033[0m" << std::endl;
    }
}

void UserChat::print_add_member(const chat::Chat& chat_msg) {
    if(chat_msg.add_member().decide()) {
        std::string user = chat_msg.add_member().username();
        std::cout << "\033[32m" << user << "已成功进群!\033[0m" << std::endl;
    } else {
        std::string msg = chat_msg.add_member().msg();
        std::cout << msg << std::endl;
    }
}

void UserChat::print_add_res(const chat::Chat& chat_msg) {
    std::string user = chat_msg.add_member_res().username();
    std::string u_name = chat_msg.add_member_res().u_name();
    std::string g_name = chat_msg.add_member_res().g_name();
   
    std::string name = g_name + "   ---" + u_name;
    std::cout << "\033[34m您已被 " << user << "拉入群 " << name << "\033[0m" << std::endl;
    user_groups.insert(name);
}

void UserChat::print_del_member(const chat::Chat& chat_msg) {
    if(chat_msg.del_member().decide()) {
        std::string user = chat_msg.add_member().username();
        std::cout << "\033[32m已成功踢出成员 " << user << " !\033[0m" << std::endl;
    } else {
        std::string msg = chat_msg.del_member().msg();
        std::cout << msg << std::endl;
    }
}

void UserChat::print_del_res(const chat::Chat& chat_msg) {
    std::string user = chat_msg.del_member_res().username();
    std::string u_name = chat_msg.del_member_res().u_name();
    std::string g_name = chat_msg.del_member_res().g_name();
   
    std::string name = g_name + "   ---" + u_name;
    std::cout << "\033[34m您已被 " << user << "踢出群聊 " << name << "\033[0m" << std::endl;
    user_groups.erase(name);
    group_exixt = false;
}

void UserChat::print_disband_group(const chat::Chat& chat_msg) {
    std::string u_name = chat_msg.disband_group().u_name();
    std::string g_name = chat_msg.disband_group().g_name();
     std::string name = g_name + "   ---" + u_name;
    std::cout << "\033[34m群聊 " << name << " 已被解散\033[0m" << std::endl;
    user_groups.erase(name);
    group_exixt = false;
}

void UserChat::print_add_manager(const chat::Chat& chat_msg) {
    std::string u_name = chat_msg.add_manager().u_name();
    std::string g_name = chat_msg.add_manager().g_name();
    std::string user = chat_msg.add_manager().username();
    std::string name = g_name + "   ---" + u_name;
    if(chat_msg.add_manager().decide()) {
        std::cout << "\033[32m已成功将 " << user << " 设为管理员!\033[0m" << std::endl;
    } else {
        std::cout << chat_msg.add_manager().msg() << std::endl;
    }
}

void UserChat::print_add_manager_res(const chat::Chat& chat_msg) {
    std::string u_name = chat_msg.add_manager().u_name();
    std::string g_name = chat_msg.add_manager().g_name();
    std::string user = chat_msg.add_manager().username();
    std::string name = g_name + "   ---" + u_name;
    std::cout << "\033[34m您已被设为群聊 " << name << " 的管理员!\033[0m" << std::endl;
    group_exixt = false;
}

void UserChat::print_del_manager(const chat::Chat& chat_msg) {
    std::string u_name = chat_msg.del_manager().u_name();
    std::string g_name = chat_msg.del_manager().g_name();
    std::string user = chat_msg.del_manager().username();
    std::string name = g_name + "   ---" + u_name;
    if(chat_msg.del_manager().decide()) {
        std::cout << "\033[32m已成功将 " << user << " 移除管理员!\033[0m" << std::endl;
    } else {
        std::cout << chat_msg.del_manager().msg() << std::endl;
    }
}

void UserChat::print_del_manager_res(const chat::Chat& chat_msg) {
    std::string u_name = chat_msg.del_manager().u_name();
    std::string g_name = chat_msg.del_manager().g_name();
    std::string user = chat_msg.del_manager().username();
    std::string name = g_name + "   ---" + u_name;
    std::cout << "\033[34m您已被移除群聊 " << name << " 的管理员身份!\033[0m" << std::endl;
    group_exixt = false;
}

void UserChat::print_send_group_file(const chat::Chat& chat_msg) {
    std::string u_name = chat_msg.send_file_group().u_name();
    std::string g_name = chat_msg.send_file_group().g_name();
    std::string name = g_name + "   ---" + u_name;
    std::string user = chat_msg.send_file_group().name();
    std::string file = chat_msg.send_file_group().file_name();
    std::string time = chat_msg.send_file_group().time();
    std::cout << "\033[1;33m您有一个未读群文件, 来自群 \033[1;32m" << name << " \033[1;33m的 \033[1;32m" << user << "\033[0m" << std::endl;
    std::cout << "\033[34m时间是:" << time << "\033[0m" << std::endl;
}

void UserChat::print_look_group_file(const chat::Chat& chat_msg) {
    std::unordered_map<std::string, std::vector<std::string>> result;
    if(chat_msg.look_fiele_req_group().result().empty()) {
        std::cout << "\033[1;33m没有群友发送文件---\033[0m" << std::endl;
        return;
    }
    for(const auto& [friend_name, friend_files] : chat_msg.look_fiele_req_group().result()) {
        for(const auto& file : friend_files.files()) {
            std::cout << friend_name << "   " <<  "\033[36m" << file << "\033[0m" << std::endl;
            this->group_files[friend_name].push_back(file);
        }
    }
    std::cout << "\033[1;34m若要处理,请输入 7;\033[0m" << std::endl;
    std::cout << "\033[1;34m若不处理,请输入 13 进行安全退出;\033[0m" << std::endl;
}

void UserChat::print_online_group_msg(const chat::Chat& chat_msg) {
    std::cout << "on" << std::endl;
    const auto& messages = chat_msg.chat_list().msg();
    for(const auto& message : messages) {
        std::cout << message << std::endl;
    } 
    if(!messages.empty()) {
        std::cout << "\033[1;34m=======以上是历史消息=======\033[0m" << std::endl;
    }
}
