#include "User_chat.h"


void UserChat::send_add_group() {
    std::string g_name;
    std::cout << "\033[34m👋请输入群聊的名字吧:\033[0m" << std::endl;
    std::getline(std::cin, g_name);
    
    chat::Chat add_f; 
    add_f.set_group(chat::Group::ADDGROUP);
    chat::AddGroup* req = add_f.mutable_add_group();
    req->set_username(_username);
    req->set_group_name(g_name);
    std::string time = Protocol::GetNowTime();
    req->set_time(time);
    std::string msg; 
    add_f.SerializeToString(&msg);
    if(!Send(msg)) {
        std::cout << "\033[31m群聊创建请求发送失败! \033[0m" << std::endl;
    }
}

void UserChat::join_group() {
    std::cout << "\033[34m请输入群聊的名字吧👉:\033[0m" << std::endl;
    std::string g_name;
    getline(std::cin, g_name);
    std::cout << "\033[34m请输入群主的名字吧👉:\033[0m" << std::endl;
    std::string username;
    getline(std::cin, username);
    std::string name = g_name + "   ---" + username;
    if(user_groups.find(name) != user_groups.end()) {
        std::cout << "\033[31m您已经加入该群聊了! \033[0m" << std::endl;
        std::cout << "\033[34m正在为您退出...\033[0m" << std::endl;
        return;
    }

    chat::Chat chat_group;
    chat_group.set_group(chat::Group::JOINGROUP);
    chat::JoinGroup* join_group = chat_group.mutable_join_group();
    join_group->set_u_name(username);
    join_group->set_g_name(g_name);
    join_group->set_username(_username);
    std::string msg;
    chat_group.SerializeToString(&msg);
    Send(msg);
}

void UserChat::group_list() {
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::GROUPLIST);
    chat::GroupList* group_list = chat_group.mutable_group_list();
    group_list->set_username(_username);
    std::string group;
    chat_group.SerializeToString(&group);
    
    Send(group);
    // std::cout << "sssss" << std::endl;
}

// 查看群聊申请
void UserChat::group_request() {
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::FINDREQ);
    std::string msg;
    chat_group.SerializeToString(&msg);
    
    Send(msg);
}

void UserChat::deicde_group_request() {
    std::cout << "\033[34m请输入群聊的名字吧👉:\033[0m" << std::endl;
    std::string g_name;
    getline(std::cin, g_name);
    std::cout << "\033[34m请输入群主的名字吧👉:\033[0m" << std::endl;
    std::string username;
    getline(std::cin, username);
    std::string name = g_name + "   ---" + username;
    if(user_groups.find(name) == user_groups.end()) {
        std::cout << "\033[31m您未加入该群聊! \033[0m" << std::endl;
        std::cout << "\033[34m正在为您退出...\033[0m" << std::endl;
        return;
    }
    std::cout << "\033[34m请输入你要处理的名字吧👉:\033[0m" << std::endl;
    std::string user;
    getline(std::cin, user);

    int decide;
    std::cout << "\033[1;37m请选择你的操作: 同意[1] 拒绝[2] 忽视[3]\033[0m" << std::endl;
    std::cin >> decide;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if(decide == 3) return;
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::JOINGROUPRESPONSE);
    chat::JoinGroupResponse* join_response = chat_group.mutable_join_res();
    join_response->set_g_name(g_name);
    join_response->set_u_name(username);
    join_response->set_username(user);
    if(decide == 1) {
        join_response->set_decide(true);
    } else if(decide == 2) {
        join_response->set_decide(false);
    }
    std::string time = Protocol::GetNowTime();
    join_response->set_time(time);
    std::string msg;
    chat_group.SerializeToString(&msg);

    Send(msg);
}

void UserChat::choice_groups() {
    group_list();
    sleep(0.5);
    std::cout << "\033[1;37m请输入您要进入的群聊名称:\033[0m" << std::endl;
    std::string g_name;
    getline(std::cin, g_name);
    std::cout << "\033[34m请输入群主的名称👉:\033[0m" << std::endl;
    std::string username;
    getline(std::cin, username);
    std::string name = g_name + "   ---" + username;
    if(user_groups.find(name) == user_groups.end()) {
        std::cout << "\033[31m您未加入该群聊! \033[0m" << std::endl;
        std::cout << "\033[34m正在为您退出...\033[0m" << std::endl;
        return;
    }

    chat::Chat chat_group;
    chat_group.set_group(chat::Group::CHOICEGROUP);
    chat::ChoiceGroup* choice_group = chat_group.mutable_group_choice();
    choice_group->set_u_name(username);
    choice_group->set_g_name(g_name);
    std::string msg;
    chat_group.SerializeToString(&msg);

    Send(msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    runGroups();
}

void UserChat::send_group_member() {
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::GROUPMEMBERS);
    chat::GroupMembers* group_member = chat_group.mutable_group_members();
    group_member->set_u_name(user_name);
    group_member->set_g_name(group_name);
    std::string msg;
    chat_group.SerializeToString(&msg);

    Send(msg);
}

void UserChat::chat_group() {
    std::cout << "\033[36m您已进入聊天, 输入<quit>即可退出...\033[0m" << std::endl;

    _running_recv = false;
    t1 = std::thread(&UserChat::send_group_msg, this);
    t2 = std::thread(&UserChat::recv_group_msg, this);
    joinThread(t1);
    std::cout << "\033[36m您已退出聊天\033[0m" << std::endl;
    _running = false;
    joinThread(t2);
    std::cout << "......" << std::endl;
    _running_recv = true;
    _running = true;

}

void UserChat::chat_group_list() {
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::CHATGROUPLIST);
    chat::GroupChatList* chat_list = chat_group.mutable_chat_list();
    chat_list->set_u_name(user_name);
    chat_list->set_g_name(group_name);
    std::string msg;
    chat_group.SerializeToString(&msg);

    Send(msg);
}

void UserChat::leave_group() {
    if(_statue == "群主") {
        std::cout << "\033[31m群主不能退出群聊,您可以选择解散群聊\033[0m" << std::endl;
        return;
    }
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::LEAVEGROUP);
    chat::LeaveGroup* leave_group = chat_group.mutable_leave_group();
    leave_group->set_u_name(user_name);
    leave_group->set_g_name(group_name);
    leave_group->set_statue(_statue);
    std::string msg;
    chat_group.SerializeToString(&msg);
    
    Send(msg);
}

void UserChat::add_group_member() {
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::ADDMEMBER);
    chat::AddMember* add_member = chat_group.mutable_add_member();
    add_member->set_u_name(user_name);
    add_member->set_g_name(group_name);
    std::string username;
    std::cout << "\033[36m请输入您好友的名字:\033[0m" << std::endl;
    while(1) {
        getline(std::cin, username);
        if(username == _username) {
            std::cout << "\033[36m你已经是群成员了---\033[0m" << std::endl;
            std::cout << "\033[36m请重新输入或退出<quit>...\033[0m" << std::endl;
            continue;
        }
        if(username == "quit") {
            std::cout << "\033[36m正在为您退出---\033[0m" << std::endl;
            return;
        }
        if(friends.find(username) != friends.end()) {
            break;
        } 
        
        std::cout << "\033[36m👤你们不是好友---\033[0m" << std::endl;
        std::cout << "\033[36m请重新输入或退出<quit>...\033[0m" << std::endl;
    }
    add_member->set_username(username);
    std::string msg;
    chat_group.SerializeToString(&msg);

    Send(msg);
}

void UserChat::del_group_member() {
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::DELMEMBER);
    chat::DelMember* del_member = chat_group.mutable_del_member();
    del_member->set_u_name(user_name);
    del_member->set_g_name(group_name);
    std::string username;
    std::cout << "\033[36m请输入群友的名字:\033[0m" << std::endl;
    getline(std::cin, username);
    del_member->set_username(username);
    del_member->set_statue(_statue); 
    
    std::string msg;
    chat_group.SerializeToString(&msg);

    Send(msg);
}

void UserChat::disband_group() {
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::DISBANDGROUP);
    chat::DisbandGroup* disband_group = chat_group.mutable_disband_group();
    disband_group->set_g_name(group_name);
    disband_group->set_u_name(user_name);
    std::string msg;
    chat_group.SerializeToString(&msg);

    Send(msg);
}

void UserChat::add_group_manager() {
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::ADDMANAGER);
    chat::AddManager* add_manager = chat_group.mutable_add_manager();
    add_manager->set_u_name(user_name);
    add_manager->set_g_name(group_name);
    std::string username;
    std::cout << "\033[36m请输入群友的名字:\033[0m" << std::endl;
    getline(std::cin, username);
    add_manager->set_username(username); 
    
    std::string msg;
    chat_group.SerializeToString(&msg);

    Send(msg);
}

void UserChat::del_group_manager() {
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::DELMANAGER);
    chat::DelManager* del_manager = chat_group.mutable_del_manager();
    del_manager->set_u_name(user_name);
    del_manager->set_g_name(group_name);
    std::string username;
    std::cout << "\033[36m请输入管理员的名字:\033[0m" << std::endl;
    getline(std::cin, username);
    del_manager->set_username(username); 
    
    std::string msg;
    chat_group.SerializeToString(&msg);

    Send(msg);
}

void UserChat::send_file_group() {
    if(user_groups.find(_name) == user_groups.end()) {
        std::cout << "\033[31m您已被群聊踢出,无法进行文件传输..." << std::endl
        << "正在为您退出...\033[0m" << std::endl;
        return;
    }
    std::cout << "\033[34m👤请输入文件路径:\033[0m" << std::endl;
    std::string path;
    getline(std::cin, path);
    if(!std::filesystem::exists(path)) {
        std::cerr << "文件路径不存在: " << path << std::endl;
        std::cout << "\033正在为您退出...\033[0m" << std::endl;
        return;
    }
    std::cout << "\033[36m文件传输中...\033[0m" << std::endl;
    std::string g_name = user_name + "---" + group_name;
    auto client = std::make_shared<FTPClient>("UPLOAD", _username, g_name, path, "");
        std::thread file1([client]() {
            client->init();
        });
    file1.detach();

    chat::Chat chat_file;
    chat_file.set_group(chat::Group::SENDFILEGROUP);
    chat::SendFileGroup* send_file_group = chat_file.mutable_send_file_group();
    send_file_group->set_g_name(group_name);
    send_file_group->set_u_name(user_name);
    std::filesystem::path f_name(path);
    std::string f = f_name.filename().string();
    send_file_group->set_file_name(f);
    std::string time = Protocol::GetNowTime();
    send_file_group->set_time(time);
    std::string msg;
    chat_file.SerializeToString(&msg);
    Send(msg);
}

void UserChat::recive_file_group() {
    std::string name;
    std::cout << "\033[34m👤请输入文件发送者的名字吧:\033[0m" << std::endl;
    getline(std::cin, name);
    if(group_files[name].empty()) {
        std::cout << "\033[34m这个群友没有发送群文件---\033[0m" << std::endl;
        return;
    }
    std::string f;
    std::cout << "\033[34m📝请输入文件的名字吧:\033[0m" << std::endl;
    getline(std::cin, f);

    auto it = group_files.find(name);
    // std::cout << *this->friends_files[name].begin() << ":123:" << *this->friends_files[name].end() << std::endl;
    for(const auto& file : this->group_files[name]) {
        std::cout << f << std::endl;
        if(f == file) {
            chat::Chat chat_file;
            chat_file.set_group(chat::Group::RECVFILEGROUP);
            chat::RecvFileGroup* recv_file = chat_file.mutable_recv_file_group();
            recv_file->set_name(name);
            recv_file->set_u_name(user_name);
            recv_file->set_g_name(group_name);
            recv_file->set_file_name(file);
            std::string buf;
            chat_file.SerializeToString(&buf);
            Send(buf);
            std::string g_name = user_name + "---" + group_name;
            auto client = std::make_shared<FTPClient>("DOWNLOAD", g_name, name, file, file);
            std::thread file2([client]() {
                client->init();
            });
            file2.detach();

            if(it != group_files.end()) {
                auto& vec = it->second;
                auto pos = std::find(vec.begin(), vec.end(), file);
                if(pos != vec.end()) {
                    vec.erase(pos);
                }
            }
            return;
        }
    }
    std::cout << "\033[1;31m没有找到这个文件\033[0m" << std::endl;
    
}

void UserChat::look_file_group() {
    chat::Chat chat_file;
    chat_file.set_group(chat::Group::LOOKFILEGROUP);
    chat::LookFileRequestGroup* look_req = chat_file.mutable_look_fiele_req_group();
    look_req->set_username(_username);
    look_req->set_u_name(user_name);
    look_req->set_g_name(group_name);
    std::string file_req;
    chat_file.SerializeToString(&file_req);
    
    Send(file_req);
}



// chat1
void UserChat::send_group_msg() {
    while(true) {
        std::string msg;
        if(user_groups.find(_name) == user_groups.end()) {
            std::cout << "\033[1;31m您已被群聊踢出,无法进行聊天..." << std::endl
            << "\033[34m正在为您退出聊天...\033[0m" << std::endl;
            group_exixt = false;
            break;
        }

        std::getline(std::cin, msg);

        if(strcmp(msg.c_str(), "quit") == 0) {
            _running = false;
            break;
        }
        
        chat::Chat chat_msg;
        chat_msg.set_group(chat::Group::GROUPMSG);
        chat::GroupChat* group_chat = chat_msg.mutable_group_chat();
        group_chat->set_u_name(user_name);
        group_chat->set_g_name(group_name);
        group_chat->set_username(_username);
        group_chat->set_msg(msg);
        // std::cout << msg << std::endl;

        std::string message;
        chat_msg.SerializeToString(&message);
        Send(message);
    }
}

// chat2
void UserChat::recv_group_msg() {
    sleep(1);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    while(_running) {
        
        char buf[MAX_NUM];
        ssize_t len = recv(_sockfd, buf, sizeof(buf), 0);
        if(!_running) break;

        if(len > 0) {
            msg.append(buf, len);
            // std::cout<< msg << std::endl;
        } else if(len == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                // std::cout<< "error" << std::endl;
                if(!_running) break;
                continue;
            } else {
                std::cerr << "Recive error:" << std::strerror(errno) << std::endl;
                break;
            }
        } else if(len == 0) {
            // std::cout<< "lll" << std::endl;
            break;
        }
        chat::Chat chat_msg;
        while(true) {
            // std::cout<< "hhh" << std::endl;
            if(!Protocol::unpack(msg, chat_msg)) break;

        switch (chat_msg.action())
            {
            case chat::Actions::LOGINLIST:
                save_friends(chat_msg);
                break;
            case chat::Actions::FRIENDMSG:
                print_msg(chat_msg);
                break;
            case chat::Actions::ADDFDREQ:
                recv_add_friend(chat_msg);
                break;
            case chat::Actions::ADDFDRES:
                print_add_friend(chat_msg);
                break;
            case chat::Actions::USERNTF:
                print_notify(chat_msg);
                break;
            case chat::Actions::FDLIST:
                print_friends(chat_msg);
                break;
            case chat::Actions::DELFDRES:
                print_delete(chat_msg);
                break;
            case chat::Actions::DELFDREQ:
                printf_delete_req(chat_msg);
                break;
            case chat::Actions::RESPONSE:
                print_response(chat_msg);
                break;
            case chat::Actions::OFFLINEMSG:
                print_offline_msg(chat_msg);
                break;
            case chat::Actions::UNLOGIN:
                print_cancel();
                break;
            case chat::Actions::BLOCKFRIEND:
                print_block(chat_msg.block_friend().name());
                break;
            case chat::Actions::UNBLOCKFRIEND:
                print_unblock(chat_msg.block_friend().name());
                break;
            case chat::Actions::SENDFILE:
                print_send_file(chat_msg);
                break;
            case chat::Actions::LOOKFILE:
                print_look_file(chat_msg);
                break;
            case chat::Actions::ACTION_QUIT:
                break;
            default:
                break;
            }
            switch(chat_msg.group()) {
            case chat::Group::ADDGROUP:
                recv_add_group(chat_msg);
                break;
            case chat::Group::JOINGROUP:
                decide_join_group(chat_msg);
                break;
            case chat::Group::JOINGROUPRESPONSE:
                print_join_group(chat_msg);
                break;
            case chat::Group::GROUPLIST:
                print_group_list(chat_msg);
                break;
            case chat::Group::FINDREQ:
                print_group_notify(chat_msg);
                break;
            case chat::Group::CHOICEGROUP: 
                recv_choice_groups(chat_msg);
                break;
            case chat::Group::GROUPMEMBERS:
                print_group_members(chat_msg);
                break;
            case chat::Group::CHATGROUPLIST:
                print_chat_group_list(chat_msg);
                break;
            case chat::Group::LEAVEGROUP:
                print_leave_group(chat_msg);
                break;
            case chat::Group::ADDMEMBER:
                print_add_member(chat_msg);
                break;
            case chat::Group::ADDMEMBERRES:
                print_add_res(chat_msg);
                break;
            case chat::Group::DELMEMBER:
                print_del_member(chat_msg);
                break;
            case chat::Group::DELMEMBERRES:
                print_del_res(chat_msg);
                break;
            case chat::Group::DISBANDGROUP:
                print_disband_group(chat_msg);
                break;
            case chat::Group::ADDMANAGER:
                print_add_manager(chat_msg);
                break;
            case chat::Group::ADDMANAGERRES:
                print_add_manager_res(chat_msg);
                break;
            case chat::Group::DELMANAGER:
                print_del_manager(chat_msg);
                break;
            case chat::Group::DELMANAGERRES:
                print_del_manager_res(chat_msg);
                break;
            case chat::Group::SENDFILEGROUP:
                print_send_group_file(chat_msg);
                break;
            case chat::Group::LOOKFILEGROUP:
                print_look_group_file(chat_msg);
                break;
            case chat::Group::GROUP_QUIT:
                break;
            default:
                break;
            }
            std::string u_name = chat_msg.group_chat().u_name();
            std::string g_name = chat_msg.group_chat().g_name();
            std::string name = g_name + "   ---" + u_name;
            if(name == _name){
                std::cout << "\033[36m" << chat_msg.group_chat().msg() << "\033[0m" << std::endl;
                continue;
            } else if(name != _name && !name.empty()){
                std::cout << "\033[1;33m您收到一条新的未读群消息,来自 \033[1;32m" << chat_msg.group_chat().username() << "\033[0m" << std::endl;
                std::cout << "\033[1;33m属于群: " << name << "\033[0m" << std::endl;
            }
        }
    }

    // std::cout<< "ppp" << std::endl;
    timeout.tv_sec = 0;
    setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}