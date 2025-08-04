#include "User_chat.h"

void UserChat::chat_friend() {
    std::cout << "\033[34m👤请输入你好友的名字吧:\033[0m" << std::endl;
    send_List();
    std::string to_name;
    bool run = true;
    bool f = false;
    while(run) {
        getline(std::cin, to_name);
        if(strcmp(to_name.c_str(), "quit") == 0) return;
        
        for(const auto& name : friends)
        {
            if(name == to_name) {
                _running_recv = false;
                run = false;
                f = true;
                break;
            }
        }
        if(!f) {
            std::cout << "\033[31m他(她)好像不是您的好友\n请重新输入或退出(quit)\033[0m" << std::endl;
        }
    }
    std::cout << "\033[36m您已进入聊天, 输入<quit>即可退出...\033[0m" << std::endl;

    t1 = std::thread(&UserChat::send_msg, this, to_name);
    t2 = std::thread(&UserChat::recive_msg, this, to_name);
    joinThread(t1);
    std::cout << "\033[36m您已退出聊天\033[0m" << std::endl;
    _running = false;
    joinThread(t2);
    std::cout << "......" << std::endl;
    _running_recv = true;
    _running = true;
}

void UserChat::send_List() {
    chat::Chat list;
    list.set_action(chat::Actions::FDLIST);
    chat::FriendLists* friends = list.mutable_friends();
    friends->set_name(_username);
    std::string list_req;
    list.SerializeToString(&list_req);

    Send(list_req);
}

void UserChat::send_add_friend() {
    std::string f_name;
    std::cout << "\033[34m👤请输入用户的名字吧:\033[0m" << std::endl;
    std::getline(std::cin, f_name);
    if(f_name == _username) {
        std::cout << "\033[31m❌不可以跟自己加好友!!! \033[0m" << std::endl;
        return;
    }
    if(friends.find(f_name) != friends.end()) {
        std::cout << "\033[36m👤你们已经是好友了---\033[0m" << std::endl;
        std::cout << "\033[36m按<Enter>退出---\033[0m" << std::endl;
        getline(std::cin, f_name);
        return;
    }
    chat::Chat add_f; 
    add_f.set_action(chat::Actions::ADDFDREQ);
    chat::AddFriendRequest* req = add_f.mutable_friend_req();
    req->set_from_username(_username);
    req->set_to_username(f_name);
    std::string time = Protocol::GetNowTime();
    req->set_time(time);
    std::string msg; 
    add_f.SerializeToString(&msg);
    if(!Send(msg)) {
        std::cout << "\033[31m此用户不存在! 检查一下自己的输入吧! \033[0m" << std::endl;
    } else {
        std::cout << "\033[34m已发送---\033[0m" << std::endl;
    }
}

void UserChat::handleReq() {
    // _running_recv = false;
    // struct timeval timeout;
    // timeout.tv_usec = 0;
    // timeout.tv_sec = 0;
    // setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    chat::Chat chat_req;
    chat_req.set_action(chat::Actions::USERNTF);
    chat::UserRequest* req = chat_req.mutable_req();
    req->set_username(_username);
    std::string _req;
    chat_req.SerializeToString(&_req);
    
    Send(_req);
    // std::cout << "\033[34m已发送---\033[0m" << std::endl;

    // char buf[MAX_NUM];
    // ssize_t len = recv(_sockfd, buf, MAX_NUM, 0);
    // std::cout << "成功" << std::endl;
    // if(len < 0) {
    //     std::cout << "error1" << std::endl;
    //     return;
    // }
    // std::string res = buf;
    // std::cout << res.size() << std::endl;
    // chat::Chat chat_msg;
    // Protocol::unpack(res, chat_msg);
    // print_notify(chat_msg);
}

void UserChat::decide_add_friend() {
    std::cout << "\033[34m👤可以输入你要处理的用户名了:\033[0m" << std::endl;
    while(true) {
        std::string name;
        getline(std::cin, name);
        if(name == "quit" || name == "QUIT") {
            break;
        }
        auto it = std::find(friend_res.begin(), friend_res.end(), name);
        if(it == friend_res.end()) {
            std::cout << "\033[31m你输入的名字不在未处理消息中." << std::endl
            << "请重新输入:\033[0m" << std::endl <<
            "\033[34m您也可以输入<quit>/<QUIT>进行退出\033[0m" << std::endl;
            continue;
        }
        std::cout << "\033[33m同意(1)  拒绝(2)  不作处理并删除(0)\033[0m"
        << "\033[33m请作出您的选择：\033[0m";
        int select;
        std::cin >> select;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if(select == 1) {
            decide_add_friend(name, select);
            friends.insert(name);
        } else if(select == 2) {
            decide_add_friend(name, select);
        } else if(select == 0) {
            std::cout << "\033[34m已删除\033[0m" << std::endl;
        }
        delete_add_friend(name);
        std::cout << "\033[34m👤您可以输入继续输入用户名称进行操作;\033[0m" << std::endl;
        std::cout << "\033[34m您也可以输入<quit>/<QUIT>进行退出\033[0m" << std::endl;
        std::string buf;
        getline(std::cin, buf);
        if(buf == "quit" || buf == "QUIT") {
            break;
        }
    }
    Clear::clearScreen();
}

void UserChat::send_delete_friend() {
    std::string to_name;
    while(true) {
        std::cout << "\033[34m请输入你要删除的好友:\033[0m" << std::endl;
        getline(std::cin, to_name);
        if(friends.find(to_name) != friends.end()) {
            friends.erase(to_name);
            chat::Chat chat_del;
            chat_del.set_action(chat::Actions::DELFDREQ);
            chat::DeleteFriendRequest* del = chat_del.mutable_friend_del_req();
            del->set_from_name(_username);
            del->set_to_name(to_name);
            std::string time = Protocol::GetNowTime();
            del->set_time(time);
            std::string del_fd;
            if(!chat_del.SerializeToString(&del_fd)) {
                std::cerr << "\033[34mSeria error!\033[0m" << std::endl;
            }
    
            Send(del_fd);
            break;
        } else {
            if(to_name == "quit" || to_name == "QUIT") {
                break;
            }
            std::cout << "\033[31m好友不存在,请重新输入!\033[0m" << std::endl;
            std::cout << "\033[34m您也可以选择输入<quit>/<QUIT>退出.\033[0m" << std::endl;
        }
    }
    
}

void UserChat::send_offline() {

    std::string name;
    std::cout << "\033[34m请输入你要查看的好友:\033[0m" << std::endl;
    bool run = true;
    bool f = false;
    while(run) {
        getline(std::cin, name);
        if(strcmp(name.c_str(), "quit") == 0) return;
        
        for(const auto& name : friends)
        {
            if(name == name) {
                run = false;
                f = true;
                break;
            }
        }
        if(!f) {
            std::cout << "\033[31m他(她)好像不是您的好友\n请重新输入或退出(quit)\033[0m" << std::endl;
        }
    }

    chat::Chat chat_off;
    chat_off.set_action(chat::Actions::OFFLINEMSG);
    chat::OfflineMSG* off_msg = chat_off.mutable_off_msg();
    off_msg->set_username(_username);
    off_msg->set_name(name);
    std::string msg;
    chat_off.SerializeToString(&msg);

    Send(msg);
}

void UserChat::unblock_friend() {
    std::string name;
    std::cout << "\033[34m请输入你要解除屏蔽的好友\033[0m" << std::endl;
    while(true) {

        getline(std::cin, name);
        if(name == "quit") {
            break;
        }

        if(friends.find(name) == friends.end()) {
            std::cout << "\033[31m他(她)好像不是您的好友\n请重新输入或退出(quit)\033[0m" << std::endl;
        } else {
            if (friends_block.find(name) == friends_block.end()) {
                std::cout << "\033[31m他(她)没有被您屏蔽\n重新输入或退出(quit)\033[0m" << std::endl;
            } else {
                friends_block.erase(name);
                friends_block.insert(name);
                chat::Chat chat_b;
                chat_b.set_action(chat::Actions::UNBLOCKFRIEND);
                chat::BlockFriend* block = chat_b.mutable_block_friend();
                block->set_name(name);
                std::string block_fd;
                chat_b.SerializeToString(&block_fd);
                Send(block_fd);
            }
            
        }

    }
}

void UserChat::block_friend() {
    std::string name;
    std::cout << "\033[34m请输入你要屏蔽的好友\033[0m" << std::endl;
    while(true) {

        getline(std::cin, name);
        if(name == "quit") {
            break;
        }

        if(friends.find(name) == friends.end()) {
            std::cout << "\033[31m他(她)好像不是您的好友\n请重新输入或退出(quit)\033[0m" << std::endl;
        } else {
            if (friends_block.find(name) != friends_block.end()) {
                std::cout << "\033[31m他(她)已被您屏蔽\n重新输入或退出(quit)\033[0m" << std::endl;
            } else {
                friends_block.insert(name);
                chat::Chat chat_b;
                chat_b.set_action(chat::Actions::BLOCKFRIEND);
                chat::BlockFriend* block = chat_b.mutable_block_friend();
                block->set_name(name);
                std::string block_fd;
                chat_b.SerializeToString(&block_fd);
                Send(block_fd);
                std::cout << "\033[1;32m屏蔽成功!\033[0m" << std::endl;
                break;
            }
            
        }

    }

}

void UserChat::send_file() {
    std::string name;
    std::cout << "\033[34m👤请输入用户的名字吧:\033[0m" << std::endl;
    while(1) {
        getline(std::cin, name);
        if(name == "quit") {
            std::cout << "\033[36m您已退出文件传输---\033[0m" << std::endl;
            return;
        }
        if(friends.find(name) != friends.end() && friends_by_block.find(name) == friends_by_block.end()) {

            break;
        } 
        if(friends_by_block.find(name) != friends_by_block.end()) {
            std::cout << "\033[31m您已被好友屏蔽,无法进行文件传输..." << std::endl
            << "正在为您退出...\033[0m" << std::endl;
            return;
        }
        std::cout << "\033[36m👤你们不是好友---\033[0m" << std::endl;
        std::cout << "\033[36m请重新输入或退出<quit>...\033[0m" << std::endl;
        
    }
    std::cout << "\033[34m👤请输入文件路径:\033[0m" << std::endl;
    std::string path;
    getline(std::cin, path);
    if(!std::filesystem::exists(path)) {
        std::cerr << "文件路径不存在: " << path << std::endl;
        return;
    }
    std::cout << "\033[36m文件传输中...\033[0m" << std::endl;
    auto client = std::make_shared<FTPClient>("UPLOAD", _username, name, path, "");
        std::thread file1([client]() {
            client->init();
        });
    file1.detach();

    chat::Chat chat_file;
    chat_file.set_action(chat::Actions::SENDFILE);
    chat::SendFile* send_file = chat_file.mutable_send_file();
    send_file->set_name(name);
    std::filesystem::path f_name(path);
    std::string f = f_name.filename().string();
    send_file->set_file_name(f);
    std::string time = Protocol::GetNowTime();
    send_file->set_time(time);
    std::string msg;
    chat_file.SerializeToString(&msg);
    Send(msg);
}

void UserChat::look_file() {
    chat::Chat chat_file;
    chat_file.set_action(chat::Actions::LOOKFILE);
    chat::LookFileRequest* look_req = chat_file.mutable_file_req();
    look_req->set_username(_username);
    std::string file_req;
    chat_file.SerializeToString(&file_req);
    
    Send(file_req);
}

void UserChat::recive_file() {
    std::string name;
    std::cout << "\033[34m👤请输入文件发送者的名字吧:\033[0m" << std::endl;
    while(1) {
        getline(std::cin, name);
        if(name == "quit") {
            std::cout << "\033[36m您已退出文件接收---\033[0m" << std::endl;
            return;
        }
        if(friends.find(name) != friends.end()) {
            break;
        } 
        
        std::cout << "\033[36m👤你们不是好友---\033[0m" << std::endl;
        std::cout << "\033[36m请重新输入或退出<quit>...\033[0m" << std::endl;
    }
    if(friends_files[name].empty()) {
        std::cout << "\033[34m这个好友没有向您发送文件---\033[0m" << std::endl;
        return;
    }

    auto it = friends_files.find(name);
    // std::cout << *this->friends_files[name].begin() << ":123:" << *this->friends_files[name].end() << std::endl;
    for(const auto& file : this->friends_files[name]) {
        std::cout << file << std::endl;

        chat::Chat chat_file;
        chat_file.set_action(chat::Actions::RECVFILE);
        chat::RecvFile* recv_file = chat_file.mutable_recv_file();
        recv_file->set_name(name);
        recv_file->set_file_name(file);
        std::string buf;
        chat_file.SerializeToString(&buf);
        Send(buf);
        
        auto client = std::make_shared<FTPClient>("DOWNLOAD", _username, name, file, file);
        std::thread file2([client]() {
            client->init();
        });
        file2.detach();
        
        if(it != friends_files.end()) {
            auto& vec = it->second;
            auto pos = std::find(vec.begin(), vec.end(), file);
            if(pos != vec.end()) {
                vec.erase(pos);
            }
        }
        break;
    }
}


// chat1
void UserChat::send_msg(const std::string& to_name) {
    while(true) {
        std::string msg;
        if(friends.find(to_name) == friends.end()) {
            std::cout << "\033[31m您已被好友删除,无法进行聊天..." << std::endl
            << "正在为您退出聊天...\033[0m" << std::endl;
            break;
        }
        if(friends_by_block.find(to_name) != friends_by_block.end()) {
            std::cout << "\033[31m您已被好友屏蔽,无法进行聊天..." << std::endl
            << "正在为您退出聊天...\033[0m" << std::endl;
            break;
        }

        std::getline(std::cin, msg);

        if(strcmp(msg.c_str(), "quit") == 0) {
            _running = false;
            break;
        }
        
        chat::Chat chat_msg;
        chat_msg.set_action(chat::Actions::FRIENDMSG);
        chat::FriendChatMessage* chat_f = chat_msg.mutable_chat_fd();
        chat_f->set_from_name(_username);
        chat_f->set_to_name(to_name);
        std::string time = Protocol::GetNowTime();
        chat_f->set_time(time);
        // std::cout << msg << std::endl;
        chat_f->set_message(msg);
        if(friends_block.find(to_name) == friends_block.end()) {
            chat_f->set_block(false);
        } else {
            chat_f->set_block(true);
        }

        std::string message;
        chat_msg.SerializeToString(&message);
        Send(message);
    }
}

// chat2
void UserChat::recive_msg(const std::string& to_name) {
    sleep(1);
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
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
        while(!msg.empty()) {
            // std::cout<< "hhh" << std::endl;
        if(!Protocol::unpack(msg, chat_msg)) break;

        switch (chat_msg.action())
            {
            case chat::Actions::LOGINLIST:
                save_friends(chat_msg);
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
            case chat::Group::GROUPMSG:
                print_group_msg(chat_msg);
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
            if(chat_msg.chat_fd().from_name() == to_name){
                std::cout << "\033[36m" << chat_msg.chat_fd().message() << "\033[0m" << std::endl;
            } else {
                std::cout << "\033[1;32m您收到一条新的未读消息,来自 " << chat_msg.chat_fd().from_name() << "\033[0m" << std::endl;
            }
        }
    }

    // std::cout<< "ppp" << std::endl;
    timeout.tv_sec = 0;
    setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

