#include "connection.h"

Connection::Connection(int fd, UserManager& user_msg)
: _fd(fd),
_user_msg(user_msg),
_redis("tcp://127.0.0.1:6379")
{}

const std::string &Connection::get_username() const
{
    return fds[_fd];
}

bool Connection::readMessage()
{
    char buffer[MAX_NUM];
    ssize_t byte_read;

    while ((byte_read = ::read(_fd, buffer, MAX_NUM)) > 0)
    {
        _buffer.append(buffer, byte_read);
        // std::cout << _buffer << "11111";
    }

    if (byte_read == 0)
    {
        std::cout << "close" << std::endl;
        return false;
    }

    if (byte_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        // std::cout << "sssss" << std::endl;
        return false;
    }
    while (1)
    {
        
        chat::Chat chat;
        if(!Protocol::unpack(_buffer, chat)) break;
        std::cout << chat.action() << std::endl;
        switch (chat.action())
        {

        case chat::Actions::LOGIN:
            adduser(chat.log().username());
            break;
        case chat::Actions::DELACCOUNT:
            del_account(chat);
            break;
        case chat::Actions::ADDFDREQ:
            send_addFriend(chat);
            break;
        case chat::Actions::ADDFDRES:
            addFriend(chat);
            break;
        case chat::Actions::USERNTF:
            send_user_notify(chat);
            break;
        case chat::Actions::FDLIST:
            send_List(chat);
            break;
        case chat::Actions::FRIENDMSG:
            Send_msg(chat);
            break;
        case chat::Actions::UNUSERNTF:
            Delete_notify(chat);
            break;
        case chat::Actions::DELFDREQ:
            Send_delete_friend(chat);
            break;
        case chat::Actions::OFFLINEMSG:
            send_offline_msg(chat);
            break;
        case chat::Actions::HEART:
            checkHeart(chat.heart());
            break;
        case chat::Actions::BLOCKFRIEND:
            block(chat);
            break;
        case chat::Actions::UNBLOCKFRIEND:
            unblock(chat);
            break;
        case chat::Actions::SENDFILE:
            send_file(chat);
            break;
        case chat::Actions::RECVFILE:
            recv_file(chat);
            break;
        case chat::Actions::LOOKFILE:
            send_file_req(chat);
            break;
        case chat::Actions::ACTION_QUIT:
            break;
        default:
            break;
        }
        
        switch (chat.group()) {
        case chat::Group::ADDGROUP:
            add_group(chat);
            break;
        case chat::Group::JOINGROUP:
            join_group(chat);
            break;
        case chat::Group::GROUPLIST:
            group_list(chat);
            break;
        case chat::Group::FINDREQ:
            group_notify(chat);
            break;
        case chat::Group::JOINGROUPRESPONSE:
            group_decide(chat);
            break;
        case chat::Group::CHOICEGROUP:
            group_choice(chat);
            break;
        case chat::Group::GROUPMSG:
            group_chat(chat);
            break;
        case chat::Group::GROUPMEMBERS:
            group_members(chat);
            break;
        case chat::Group::CHATGROUPLIST:
            group_chat_list(chat);
            break;
        case chat::Group::LEAVEGROUP:
            leave_group(chat);
            break;
        case chat::Group::ADDMEMBER:
            add_member(chat);
            break;
        case chat::Group::DELMEMBER:
            del_member(chat);
            break;
        case chat::Group::DISBANDGROUP:
            disband_froup(chat.disband_group().g_name(), chat.disband_group().u_name());
            break;
        case chat::Group::ADDMANAGER:
            add_manager(chat);
            break;
        case chat::Group::DELMANAGER:
            del_manager(chat);
            break;
        case chat::Group::SENDFILEGROUP:
            send_file_group(chat);
            break;
        case chat::Group::RECVFILEGROUP:
            recv_file_group(chat);
            break;
        case chat::Group::LOOKFILEGROUP:
            send_file_req_group(chat);
            break;
        case chat::Group::GROUP_QUIT:
            break;
        default:
            break;
        }
    }
    return true;
}

// 注销账号
void Connection::del_account(const chat::Chat& chat) {
    std::unordered_map<std::string, std::string> groups;
    std::vector<std::string> friends;
    std::string username = fds[_fd];
    for(const auto& [g_name, u_name] : chat.del_account().group_name()) {
        std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
        const std::string status = _redis.getGroupManager(uuid, username);
        if(status == "群主") {
            disband_froup(g_name, u_name);
            
        } else {
            std::string ll = username + ":成员";
            _redis.removeGroupMember(uuid, ll);
            _redis.removeGroupManager(uuid, username);
        }
    }
    for(const auto& friend_name : chat.del_account().friends()) {
        std::string time = Protocol::GetNowTime();
        _user_msg.deleteFriend(username, friend_name);

        std::string pp;
        if(username > friend_name) {
            pp = username + friend_name;
        } else {
            pp = friend_name + username;
        }
        _redis.deleteChatList("Friend", pp);
    
        std::string r = username + "\033[31m 已注销，你们不再是好友了！\033[0m";
        int to_fd = users[friend_name];
        if(to_fd == 0) {
            std::unordered_map<std::string, std::string> message;
            message = {
                {r, time}
            };
            _redis.setResKey("Response", friend_name, message);
        } else {
            chat::Chat chat_req;
            chat_req.set_action(chat::Actions::DELFDREQ);
            chat::DeleteFriendRequest* del_req = chat_req.mutable_friend_del_req();
            del_req->set_from_name(username);
            del_req->set_to_name(friend_name);
            del_req->set_time(time);

            std::string d;
            chat_req.SerializeToString(&d);
            d = Protocol::pack(d);
            MessageCenter::instance().dispatch(_fd, to_fd, d);
        }
    }
    _redis.deleteHashMembers("Friend", username);
    _redis.deleteHashKey(username);
}



// 以下是关于群聊

// 列出文件目录
void Connection::send_file_req_group(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string username = fds[_fd];
    std::string u_name = chat.look_fiele_req_group().u_name();
    std::string g_name = chat.look_fiele_req_group().g_name();
    std::string name = g_name + "   ---" + u_name;
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    std::unordered_map<std::string, std::vector<std::string>> files;
    files = _redis.getAllFile(uuid, username);
    chat::Chat chat_files;
    chat_files.set_group(chat::Group::LOOKFILEGROUP);
    chat::LookFileRequestGroup* look_files = chat_files.mutable_look_fiele_req_group();
    for(const auto& [user_friend, friend_files] : files) {
        auto& file_lists = (*look_files->mutable_result())[user_friend];
        for(const auto& list : friend_files) {
            file_lists.add_files(list);
        }
    }

    std::string msg;
    chat_files.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd,_fd, msg);
}

// 接收文件
void Connection::recv_file_group(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string name = chat.recv_file_group().name();
    std::string u_name = chat.recv_file_group().u_name();
    std::string g_name = chat.recv_file_group().g_name();
    // std::string name = g_name + "   ---" + u_name;
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    std::string file = chat.recv_file_group().file_name();
    std::string username = fds[_fd];
    _redis.delUserFile(uuid, name, username, file);
}

// 发送文件
void Connection::send_file_group(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string username = fds[_fd];
    std::string u_name = chat.send_file_group().u_name();
    std::string g_name = chat.send_file_group().g_name();
    std::string name = g_name + "   ---" + u_name;
    std::string file_name = chat.send_file_group().file_name();
    std::string time = Protocol::GetNowTime();
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    std::vector<std::string> members;
    members = _redis.getGroupMember(uuid);
    for(const auto& member : members) {
        size_t pos = member.rfind(':');
        std::string a = (pos != std::string::npos) ? member.substr(0, pos) : member;
        if(a == username) continue;
        _redis.setUserFile(uuid, username, a, file_name);
        int fd = users[a];
        if(fd == 0) {
            std::unordered_map<std::string, std::string> msg;
            std::string b = "\033[34m您收到一个文件 " + file_name + ",来自 " + name + " 的 " + username + "\033[0m";
            msg = {
                {b, time}
            };
            _redis.setResKey("Response", a, msg);
            
            continue;
        }
        chat::Chat chat_file;
        chat_file.set_group(chat::Group::SENDFILEGROUP);
        chat::SendFileGroup* send_file_group = chat_file.mutable_send_file_group();
        send_file_group->set_name(username);
        send_file_group->set_u_name(u_name);
        send_file_group->set_g_name(g_name);
        send_file_group->set_file_name(file_name);
        send_file_group->set_time(time);
        std::string msg;
        chat_file.SerializeToString(&msg);
        msg = Protocol::pack(msg);

        MessageCenter::instance().dispatch(_fd, fd, msg);
    }

}


// 删除管理员
void Connection::del_manager(const chat::Chat& chat) {
    std::string u = fds[_fd];
    std::string u_name = chat.del_manager().u_name();
    std::string g_name = chat.del_manager().g_name();
    std::string username = chat.del_manager().username();
    std::string name = g_name + "   ---" + u_name;
    std::string time = Protocol::GetNowTime();
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    
    chat::Chat chat_add;
    chat_add.set_group(chat::Group::DELMANAGER);
    chat::DelManager* del_manager = chat_add.mutable_del_manager();
    del_manager->set_g_name(g_name);
    del_manager->set_u_name(u_name);
    del_manager->set_username(username);
    
    if(_redis.removeGroupManager(uuid, username)) {
        std::string l = username + ":管理员";
        std::string ll = username + ":成员";
        _redis.removeGroupMember(uuid, l);
        _redis.SetGroupMember(uuid, ll);
        del_manager->set_decide(true);
        int fd = users[username];
        if(fd == 0) {
            std::unordered_map<std::string, std::string> msg;
            std::string a = "\033[1;33m您已被" + u + "移除 " + g_name + "   ---" + u_name + " 管理员\033[0m";
            msg = {
                {a, time} 
            };
            _redis.setResKey("Response", username, msg);
            
        } else {
            chat::Chat chat_add;
            chat_add.set_group(chat::Group::DELMANAGERRES);
            chat::DelManager* del_res = chat_add.mutable_del_manager();
            del_res->set_g_name(g_name);
            del_res->set_u_name(u_name);
            del_res->set_username(u);
            std::string n;
            chat_add.SerializeToString(&n); 
            n = Protocol::pack(n);
            MessageCenter::instance().dispatch(_fd, fd, n);
        }
    } else {
        del_manager->set_decide(false);
        std::string msg = "\033[1;33m他(她)不在群聊中或者他(她)不是管理员!\033[0m";
        del_manager->set_msg(msg);
    }
    std::string n;
    chat_add.SerializeToString(&n);
    n = Protocol::pack(n);
    MessageCenter::instance().dispatch(_fd, _fd, n);
}

// 添加管理员
void Connection::add_manager(const chat::Chat& chat) {
    std::string u = fds[_fd];
    std::string u_name = chat.add_manager().u_name();
    std::string g_name = chat.add_manager().g_name();
    std::string username = chat.add_manager().username();
    std::string name = g_name + "   ---" + u_name;
    std::string time = Protocol::GetNowTime();
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    
    chat::Chat chat_add;
    chat_add.set_group(chat::Group::ADDMANAGER);
    chat::AddManager* add_manager = chat_add.mutable_add_manager();
    add_manager->set_g_name(g_name);
    add_manager->set_u_name(u_name);
    add_manager->set_username(username);
    
    std::unordered_map<std::string, std::string> g;
    g = {
        {username, "管理员"}
    };
    if(_redis.SetGroupManager(uuid, g)) {
        std::string l = username + ":管理员";
        std::string ll = username + ":成员";
        _redis.removeGroupMember(uuid, ll);
        _redis.SetGroupMember(uuid, l);
        add_manager->set_decide(true);
        int fd = users[username];
        if(fd == 0) {
            std::unordered_map<std::string, std::string> msg;
            std::string a = "\033[1;33m您已被" + u + "设为 " + g_name + "   ---" + u_name + " 的管理员\033[0m";
            msg = {
                {a, time} 
            };
            _redis.setResKey("Response", username, msg);
            
        } else {
            chat::Chat chat_add;
            chat_add.set_group(chat::Group::ADDMANAGERRES);
            chat::AddManager* add_res = chat_add.mutable_add_manager();
            add_res->set_g_name(g_name);
            add_res->set_u_name(u_name);
            add_res->set_username(u);
            std::string n;
            chat_add.SerializeToString(&n); 
            n = Protocol::pack(n);
            MessageCenter::instance().dispatch(_fd, fd, n);
        }
    } else {
        add_manager->set_decide(false);
        std::string msg = "\033[1;33m他(她)不在群聊中或者他(她)已经是管理员了!\033[0m";
        add_manager->set_msg(msg);
    }
    std::string n;
    chat_add.SerializeToString(&n);
    n = Protocol::pack(n);
    MessageCenter::instance().dispatch(_fd, _fd, n);
}

// 解散群聊
void Connection::disband_froup(const std::string& g_name, const std::string& u_name) {
    std::string name = g_name + "   ---" + u_name;
    std::string time = Protocol::GetNowTime();
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::DISBANDGROUP);
    chat::DisbandGroup* diaband_group = chat_group.mutable_disband_group();
    std::vector<std::string> members;
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    members = _redis.getGroupMember(uuid);
    for(const auto& member : members) {
        size_t pos = member.rfind(':');
        std::string mem = (pos != std::string::npos) ? member.substr(0, pos) : member;
        int fd = users[mem];
        if(fd == 0) {
            std::unordered_map<std::string, std::string> msg;
            std::string a = "\033[1;33m群聊 " + g_name + "   ---" + u_name + " 已解散\033[0m";
            msg = {
                {a, time}
            };
            _redis.setResKey("Response", mem, msg);
            
        } else {
            chat::Chat chat_del;
            chat_del.set_group(chat::Group::DISBANDGROUP);
            chat::DisbandGroup* del_res = chat_del.mutable_disband_group();
            del_res->set_g_name(g_name);
            del_res->set_u_name(u_name);
            std::string n;
            chat_del.SerializeToString(&n);
            n = Protocol::pack(n);
            MessageCenter::instance().dispatch(_fd, fd, n);
        }

    }
    _redis.delGroupManager(uuid);
    _redis.delGroupMember(uuid);

}

// 踢人
void Connection::del_member(const chat::Chat& chat) {
    std::string u = fds[_fd];
    std::string user = chat.del_member().username();
    std::string username1 = user + ":成员";/////
    std::string username2 = user + ":管理员";
    std::string u_name = chat.del_member().u_name();
    std::string g_name = chat.del_member().g_name();
    std::string statue = chat.del_member().statue();
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    std::string time = Protocol::GetNowTime();
    std::string w;
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::DELMEMBER);
    chat::DelMember* del_member = chat_group.mutable_del_member();
    del_member->set_username(user);
    if(statue == "群主") {
        if(_redis.removeGroupMember(uuid, username1) || _redis.removeGroupMember(uuid, username2)) {
            int fd = users[user];
            if(fd == 0) {
                std::unordered_map<std::string, std::string> msg;
                std::string a = "\033[1;33m您已被" + u + "移出群聊 " + g_name + "   ---" + u_name + "\033[0m";
                msg = {
                    {a, time}
                };
                _redis.setResKey("Response", user, msg);
                
            } else {
                chat::Chat chat_del;
                chat_del.set_group(chat::Group::DELMEMBERRES);
                chat::DelMemberResponse* del_res = chat_del.mutable_del_member_res();
                del_res->set_g_name(g_name);
                del_res->set_u_name(u_name);
                del_res->set_username(u);
                std::string n;
                chat_del.SerializeToString(&n);
                n = Protocol::pack(n);
                MessageCenter::instance().dispatch(_fd, fd, n);
            }
                _redis.UserRemoveGroups(user, g_name, u_name);
                _redis.removeGroupManager(uuid, user);
                del_member->set_decide(true);
            } else {
                del_member->set_decide(false);
                w = "\033[31m他(她)不是群成员!\033[0m";
                del_member->set_msg(w);
            }
    } else if(statue == "管理员") {
        std::string p = _redis.getGroupManager(uuid, user);
        if(p == "群主" || p == "管理员") {
            del_member->set_decide(false);
            w = "\033[31m你无权删除他(她)!\033[0m";
            del_member->set_msg(w);
            std::string msg;
            chat_group.SerializeToString(&msg);
            msg = Protocol::pack(msg);

            MessageCenter::instance().dispatch(_fd, _fd, msg);
            return;
        }
        if(_redis.removeGroupMember(uuid, username1)) {
            int fd = users[user];
            if(fd == 0) {
                std::unordered_map<std::string, std::string> msg;
                std::string a = "\033[1;33m您已被" + u + "移出群聊 " + g_name + "   ---" + u_name + "\033[0m";
                msg = {
                    {a, time}
                };
                _redis.setResKey("Response", user, msg);
                
            } else {
                chat::Chat chat_del;
                chat_del.set_group(chat::Group::DELMEMBERRES);
                chat::DelMemberResponse* del_res = chat_del.mutable_del_member_res();
                del_res->set_g_name(g_name);
                del_res->set_u_name(u_name);
                del_res->set_username(u);
                std::string n;
                chat_del.SerializeToString(&n);
                n = Protocol::pack(n);
                MessageCenter::instance().dispatch(_fd, fd, n);
            }
                _redis.UserRemoveGroups(user, u_name, g_name);
                _redis.removeGroupManager(uuid, user);
                del_member->set_decide(true);
            } else {
                del_member->set_decide(false);
                w = "\033[31m他(她)不是群成员!\033[0m";
                del_member->set_msg(w);
            }
    }
    
    std::string msg;
    chat_group.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, _fd, msg);
}

// 拉人
void Connection::add_member(const chat::Chat& chat) {
    std::string u = fds[_fd];
    std::string user = chat.add_member().username();
    std::string username = user + ":成员";
    std::string u_name = chat.add_member().u_name();
    std::string g_name = chat.add_member().g_name();
    std::string time = Protocol::GetNowTime();
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    std::string w;
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::ADDMEMBER);
    chat::AddMember* add_member = chat_group.mutable_add_member();
    add_member->set_username(user);
    if(_redis.SetGroupMember(uuid, username)) {
        int fd = users[user];
        if(fd == 0) {
            std::unordered_map<std::string, std::string> msg;
            std::string a = "\033[1;33m您已被" + u + "拉入群聊 " + g_name + "   ---" + u_name + "\033[0m";
            msg = {
                {a, time}
            };
            _redis.setResKey("Response", user, msg);
            
        } else {
            chat::Chat chat_add;
            chat_add.set_group(chat::Group::ADDMEMBERRES);
            chat::AddMemberResponse* add_res = chat_add.mutable_add_member_res();
            add_res->set_g_name(g_name);
            add_res->set_u_name(u_name);
            add_res->set_username(u);
            std::string n;
            chat_add.SerializeToString(&n);
            n = Protocol::pack(n);
            MessageCenter::instance().dispatch(_fd, fd, n);
        }
        _redis.UserSetGroups(user, u_name, g_name);
        add_member->set_decide(true);
    } else {
        add_member->set_decide(false);
        w = "\033[31m他(她)已经是群成员了!\033[0m";
        add_member->set_msg(w);
    }
    std::string msg;
    chat_group.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, _fd, msg);
}

// 退出群聊
void Connection::leave_group(const chat::Chat& chat) {
    std::string username = fds[_fd];
    std::string statue = chat.leave_group().statue();
    std::string u_name = chat.leave_group().u_name();
    std::string g_name = chat.leave_group().g_name();
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::LEAVEGROUP);
    chat::LeaveGroup*  leave_group = chat_group.mutable_leave_group();
    std::string m = username + ":" + statue;
    // std::cout << m << std::endl;
    if(!_redis.removeGroupMember(uuid, m)) {
        leave_group->set_decide(false);
        std::cout << "false" << std::endl;
    } else {
        leave_group->set_decide(true);
        std::unordered_map<std::string, std::string> managers;
        managers = _redis.getGroupManager(uuid);
        for(const auto& [user, statue] : managers) {
            if(user == username) {
                _redis.removeGroupManager(uuid, username);
            }
        }
        _redis.UserRemoveGroups(username, g_name, u_name);
        // std::cout << "true" << std::endl;
    }
    std::string msg;
    chat_group.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, _fd, msg);
}

// 群聊历史消息
void Connection::group_chat_list(const chat::Chat& chat) {
    std::string u_name = chat.chat_list().u_name();
    std::string g_name = chat.chat_list().g_name();
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    std::string username = fds[_fd];
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::CHATGROUPLIST);
    chat::GroupChatList* chat_list = chat_group.mutable_chat_list();

    std::vector<std::string> messages = _redis.getChatList("group", uuid);
    for(const auto& message : messages) {
        chat_list->add_msg(message); 
    }
    std::string msg;
    chat_group.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, _fd, msg);
} 

// 群聊
void Connection::group_chat(const chat::Chat& chat) {

    // std::lock_guard<std::mutex> lock(_mtx);
    std::string u_name = chat.group_chat().u_name();
    std::string g_name = chat.group_chat().g_name();
    std::string msg = chat.group_chat().msg();
    std::string time = Protocol::GetNowTime();
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    std::string username = fds[_fd];
    msg = username + ":" + msg;
    std::string u = uuid;
    _redis.setChatList("group", u, msg);
    std::string name = g_name + "   ---" + u_name;
    std::vector<std::string> members;
    members = _redis.getGroupMember(uuid);
    for(const auto& member : members) {
        size_t pos = member.rfind(':');
        std::string user = (pos != std::string::npos) ? member.substr(0, pos) : member;
        if(user == username) continue;
        chat::Chat chat_group;
        chat_group.set_group(chat::Group::GROUPMSG);
        chat::GroupChat* join_group = chat_group.mutable_group_chat();
        join_group->set_u_name(u_name);
        join_group->set_g_name(g_name);
        join_group->set_username(username);
        join_group->set_msg(msg);
        std::string group;
        chat_group.SerializeToString(&group);
        group = Protocol::pack(group);
        

        int fd = users[user];
        if(fd == 0) {
            std::cout << "sssss" << fd << std::endl;
            std::unordered_map<std::string, std::string> msg;
            std::string a = "\033[33m您有新的未读群消息 ,来自 \033[1;32m" + username + "\033[33m群聊:\033[1;32m" + name + "\033[0m" ;
            msg = {
                {a, time}
            };
            _redis.setResKey("Response", user, msg);
            continue;
        }
        
        MessageCenter::instance().dispatch(_fd, fd, group);
    }

}

// 列出群聊人员
void Connection::group_members(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    // std::cout << "member" << std::endl;
    std::string u_name = chat.group_members().u_name();
    std::string g_name = chat.group_members().g_name();
    std::string username = fds[_fd];
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    // std::cout << uuid << std::endl;
    std::vector<std::string> members;
    members = _redis.getGroupMember(uuid);

    chat::Chat chat_group;
    chat_group.set_group(chat::Group::GROUPMEMBERS);
    chat::GroupMembers* group_members = chat_group.mutable_group_members();

    for(const auto& member : members) {
        // std::cout << member << std::endl;
        size_t pos = member.rfind(':');
        std::string a = (pos != std::string::npos) ? member.substr(0, pos) : member;
        std::string b = member.substr(pos + 1);
        // std::cout << "a=" << a << "b=" << b << std::endl;
        (*group_members->mutable_members())[a] = b;
    }
    std::string msg;
    chat_group.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, _fd, msg);
}

// 进入哪个群聊
void Connection::group_choice(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string u_name = chat.group_choice().u_name();
    std::string g_name = chat.group_choice().g_name();
    std::string username = fds[_fd];
    
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::CHOICEGROUP);
    chat::ChoiceGroup* choice_group = chat_group.mutable_group_choice();
    choice_group->set_u_name(u_name);
    choice_group->set_g_name(g_name);
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);

    std::unordered_map<std::string, std::string> managers;
    managers = _redis.getGroupManager(uuid);
    bool decide = false;
    for(const auto& [user, statue] : managers) {
        if(user == username) {
            choice_group->set_statue(statue);
            decide = true;
        }
    }
    if(!decide) {
        choice_group->set_statue("成员");
    }
    std::string msg;
    chat_group.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, _fd, msg);
}

// 发送加群回应
void Connection::group_decide(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string u_name = chat.join_res().u_name();
    std::string g_name = chat.join_res().g_name();
    std::string username = fds[_fd];
    std::string user = chat.join_res().username();
    std::string name = g_name + "   ---" + u_name;
    std::string time = Protocol::GetNowTime();
    bool decide;
    std::string a;

    if(chat.join_res().decide()) {
        if(!_redis.userFieldHexists(user, "password")) {
            a = "\033[1;31m\n用户已注销或不存在\033[0m";
            decide = false;
            chat::Chat chat_group;
            chat_group.set_group(chat::Group::JOINGROUPRESPONSE);
            chat::JoinGroupResponse* join_res = chat_group.mutable_join_res();
            join_res->set_u_name(u_name);
            join_res->set_g_name(g_name);
            join_res->set_time(time);
            join_res->set_decide(decide);
            join_res->set_msg(a);
            std::string msg;
            chat_group.SerializeToString(&msg);
            msg = Protocol::pack(msg);

            MessageCenter::instance().dispatch(_fd, _fd, msg);
            return;
        }
        std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
        std::string n = user + ":" + "成员";
        _redis.SetGroupMember(uuid, n);
        _redis.UserSetGroups(user, u_name, g_name);
        std::unordered_map<std::string, std::string> managers;
        managers = _redis.getGroupManager(uuid);
        for(const auto& [user, statue] : managers) {
            _redis.delGroupNotify(user, user);
        }
        a = "\033[33m您被通过了加群申请 ,来自 \033[1;32m" + username + "\033[33m 您现在是 \033[1;32m" + name + " 中的一员了!\033[0m";
        decide = true;
    } else {
        a = "\033[31m您被拒绝了加群申请 ,来自 \033[1;32m" + username + "\033[33m 群聊: \033[1;32m" + name  + "\033[0m";
        decide = false;
    }
    
    if(!_redis.GroupExists(u_name, g_name)) {
        a = "\033[1;31m\n群聊不存在了\033[0m";
        decide = false;
    }

    int fd = users[user];
    if(fd == 0) {
        std::unordered_map<std::string, std::string> msg;
        msg = {
            {a, time}
        };
        _redis.setResKey("Response", user, msg);
        return;
    }

    chat::Chat chat_group;
    chat_group.set_group(chat::Group::JOINGROUPRESPONSE);
    chat::JoinGroupResponse* join_res = chat_group.mutable_join_res();
    join_res->set_u_name(u_name);
    join_res->set_g_name(g_name);
    join_res->set_time(time);
    join_res->set_decide(decide);
    join_res->set_msg(a);
    std::string msg;
    chat_group.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, fd, msg);
}

void Connection::group_notify(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string username = fds[_fd];
    chat::Chat chat_group;
    chat_group.set_group(chat::Group::FINDREQ);
    chat::FindRequest* group_notify = chat_group.mutable_group_notify();
    std::unordered_map<std::string, std::string> notify;
    notify = _redis.getGroupNotify(username);
    for(const auto& [user, value] : notify) {
        (*group_notify->mutable_response())[user] = value;
    }
    std::string msg;
    chat_group.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, _fd, msg);
}

void Connection::group_list(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string username = chat.group_list().username();
    chat::Chat chat_groups;
    chat_groups.set_group(chat::Group::GROUPLIST);
    chat::GroupList* group_list = chat_groups.mutable_group_list();
    std::vector<std::string> groups;
    groups = _redis.getGroupList(username);
    for(const auto& group_name : groups) {
        group_list->add_groups(group_name);
    }
    std::string buf;
    chat_groups.SerializeToString(&buf);
    buf = Protocol::pack(buf);

    MessageCenter::instance().dispatch(_fd, _fd, buf);
}

void Connection::join_group(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string u_name = chat.join_group().u_name();
    std::string g_name = chat.join_group().g_name();
    std::string username = fds[_fd];
    
    if(!_redis.GroupExists(u_name, g_name)) {
        chat::Chat chat_group;
        std::string msg = "\033[1;31m原因: 群聊不存在\033[0m";
        chat_group.set_group(chat::Group::JOINGROUPRESPONSE);
        chat::JoinGroupResponse* group_res = chat_group.mutable_join_res();
        group_res->set_u_name(u_name);
        group_res->set_g_name(g_name);
        group_res->set_username(username);
        group_res->set_decide(false);
        group_res->set_msg(msg);
        std::string buf;
        chat_group.SerializeToString(&buf);
        buf = Protocol::pack(buf);

        MessageCenter::instance().dispatch(_fd, _fd, buf);
        return;
    }
    
    std::string time = Protocol::GetNowTime();
    std::string uuid = _user_msg.getGroup_uuid(u_name, g_name);
    std::unordered_map<std::string, std::string> managers;
    managers = _redis.getGroupManager(uuid);
    for(const auto& [user, statue] : managers) {
        chat::Chat chat_group;
        chat_group.set_group(chat::Group::JOINGROUP);
        chat::JoinGroup* join_group = chat_group.mutable_join_group();
        join_group->set_u_name(u_name);
        join_group->set_g_name(g_name);
        join_group->set_username(username);
        join_group->set_time(time);
        std::string group;
        chat_group.SerializeToString(&group);
        group = Protocol::pack(group);
        std::unordered_map<std::string, std::string> g_info;
        
        g_info = {
            {username, group}
        };
        _redis.setGroupNotify(user, g_info);

        int fd = users[user];
        if(fd == 0) {
            std::unordered_map<std::string, std::string> msg;
            std::string a = "\033[33m您收到一条加群申请 ,来自 \033[1;32m" + username + "\033[0m";
            msg = {
                {a, time}
            };
            _redis.setResKey("Response", user, msg);
            continue;
        }
        
        MessageCenter::instance().dispatch(_fd, fd, group);
    }
}

void Connection::add_group(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string g_name = chat.add_group().group_name();
    std::string username = chat.add_group().username();
    std::string time = Protocol::GetNowTime();

    chat::Chat chat_group;
    chat_group.set_group(chat::Group::ADDGROUP);
    chat::AddGroup* add_group = chat_group.mutable_add_group();
    add_group->set_time(time);

    if(_redis.GroupExists(username, g_name)) {
        std::string msg = "\033[31m已创建过此群聊❌\033[0m";
        add_group->set_success(false);
        add_group->set_msg(msg);
    } else {
        std::string uuid = Protocol::generate_uuid();
        std::unordered_map<std::string, std::string> g_info;
        g_info = {
            {"username", username},
            {"groupname", g_name},
            {"ttime",  time}
        };
        _redis.addGroup(uuid, username, g_name, g_info);
        std::string n = username + ":" + "群主";
        _redis.SetGroupMember(uuid, n);
        _redis.UserSetGroups(username, username, g_name);
        std::unordered_map<std::string, std::string> g;
        g = {
            {username, "群主"}
        };
        _redis.SetGroupManager(uuid, g);
        std::string msg = "\033[32m群聊创建成功!\033[0m";
        add_group->set_msg(msg);
        add_group->set_username(username);
        add_group->set_group_name(g_name);
        add_group->set_success(true);
    }
    
    std::string group;
    chat_group.SerializeToString(&group);
    group = Protocol::pack(group);

    MessageCenter::instance().dispatch(_fd, _fd, group);
}

// 以下是关于好友以及部分登陆
void Connection::recv_file(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string name = chat.recv_file().name();
    std::string username = fds[_fd];
    _redis.delUserFile(username, name);
}

void Connection::send_file_req(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string username = fds[_fd];
    std::unordered_map<std::string, std::vector<std::string>> files;
    files = _redis.getAllFile(username);
    chat::Chat chat_files;
    chat_files.set_action(chat::Actions::LOOKFILE);
    chat::LookFileRequest* look_files = chat_files.mutable_file_req();
    for(const auto& [user_friend, friend_files] : files) {
        auto& file_lists = (*look_files->mutable_result())[user_friend];
        for(const auto& list : friend_files) {
            file_lists.add_files(list);
        }
    }

    std::string msg;
    chat_files.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd,_fd, msg);
}

void Connection::send_file(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string username = fds[_fd];
    std::string name = chat.send_file().name();
    std::string file_name = chat.send_file().file_name();
    std::unordered_map<std::string, std::string> user_file;
    std::string time = Protocol::GetNowTime();
    user_file = {
        {username, file_name}
    };
    _redis.setUserFile(name, username, file_name);
    int fd = users[name];
    if(fd == 0) {
        std::unordered_map<std::string, std::string> msg;
        std::string a = "\033[34m您收到一个文件 " + file_name + ",来自 " + username + "\033[0m";
        msg = {
            {a, time}
        };
        _redis.setResKey("Response", name, msg);
        
        return;
    }
    chat::Chat chat_file;
    chat_file.set_action(chat::Actions::SENDFILE);
    chat::SendFile* send_file = chat_file.mutable_send_file();
    send_file->set_name(username);
    send_file->set_file_name(file_name);
    send_file->set_time(time);
    std::string msg;
    chat_file.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, fd, msg);

}

void Connection::unblock(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string username = fds[_fd];
    std::string name = chat.block_friend().name();
    int fd = users[name];
    if(fd == 0) {
        std::unordered_map<std::string, std::string> msg;
        std::string a = username + "\033[32m已将您解除屏蔽\033[0m";
        std::string time = Protocol::GetNowTime();
        msg = {
            {a, time}
        };
        _redis.setResKey("Response", name, msg);
        
        return;
    }
    chat::Chat chat_b;
    chat_b.set_action(chat::Actions::UNBLOCKFRIEND);
    chat::BlockFriend* block = chat_b.mutable_block_friend();
    block->set_name(username);
    std::string msg;
    chat_b.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, fd, msg);
}

void Connection::block(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string username = fds[_fd];
    std::string name = chat.block_friend().name();
    int fd = users[name];
    if(fd == 0) {
        std::unordered_map<std::string, std::string> msg;
        std::string a = username + "\033[31m已将您屏蔽❌\033[0m";
        std::string time = Protocol::GetNowTime();
        msg = {
            {a, time}
        };
        _redis.setResKey("Response", name, msg);
        
        return;
    }
    chat::Chat chat_b;
    chat_b.set_action(chat::Actions::BLOCKFRIEND);
    chat::BlockFriend* block = chat_b.mutable_block_friend();
    block->set_name(username);
    std::string msg;
    chat_b.SerializeToString(&msg);
    msg = Protocol::pack(msg);

    MessageCenter::instance().dispatch(_fd, fd, msg);
}

void Connection::send_offline_msg(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string name = chat.off_msg().name();
    std::string username = chat.off_msg().username();
    std::string user;
    if(name > username) {
        user = name +":" + username;
    } else {
        user = username +":" + name;
    }
    std::vector<std::string> off_lines;
    off_lines = _redis.getChatList("Friend", user);
    chat::Chat chat_off;
    chat_off.set_action(chat::Actions::OFFLINEMSG);
    chat::OfflineMSG* off_msg = chat_off.mutable_off_msg();
    for(const auto& it : off_lines) {
        off_msg->add_msg(it);
    }
    std::string str;
    chat_off.SerializeToString(&str);
    str = Protocol::pack(str);
    
    MessageCenter::instance().dispatch(_fd, _fd, str);
}

void Connection::Send_delete_friend(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string from_name = chat.friend_del_req().from_name();
    std::string to_name = chat.friend_del_req().to_name();
    std::string time = Protocol::GetNowTime();

    chat::Chat chat_res;
    chat_res.set_action(chat::Actions::DELFDRES);
    chat::DeleteFriendResponse* del_res = chat_res.mutable_friend_del_res();
    
    if(_user_msg.deleteFriend(from_name, to_name)) {
        
        del_res->set_result(true);
        std::string pp;
        if(from_name > to_name) {
            pp = from_name + to_name;
        } else {
            pp = to_name + from_name;
        }
        _redis.deleteChatList("Friend", pp);
    
        std::string r = from_name + "\033[31m 已将您删除好友！\033[0m";
        int to_fd = users[to_name];
        if(to_fd == 0) {
            std::unordered_map<std::string, std::string> message;
            message = {
                {r, time}
            };
            _redis.setResKey("Response", to_name, message);
        } else {
            chat::Chat chat_req;
            chat_req.set_action(chat::Actions::DELFDREQ);
            chat::DeleteFriendRequest* del_req = chat_req.mutable_friend_del_req();
            del_req->set_from_name(from_name);
            del_req->set_to_name(to_name);
            del_req->set_time(time);

            std::string d;
            chat_req.SerializeToString(&d);
            d = Protocol::pack(d);
            MessageCenter::instance().dispatch(_fd, to_fd, d);
        }
    } else {
        del_res->set_result(false);
    }
    std::string del_fd;
    if(!chat_res.SerializeToString(&del_fd)) {
        std::cerr << "\033[34mSeria error!\033[0m" << std::endl;
    }
    del_fd = Protocol::pack(del_fd);
    MessageCenter::instance().dispatch(_fd, _fd, del_fd);
    
}

void Connection::send_List(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string username = chat.friends().name();
    std::vector<std::string> friends;
    friends = _user_msg.getFriend(username);
    chat::Chat chat_list;
    chat_list.set_action(chat::Actions::FDLIST);
    chat::FriendLists* lists = chat_list.mutable_friends();
    for(const auto& it : friends) {
        std::cout << it << std::endl;
        int user = users[it];
        if(user != 0) {
            (*lists->mutable_friends())[it] = true;
        } else {
            (*lists->mutable_friends())[it] = false;
        }
    }
    std::string list;
    if(!chat_list.SerializeToString(&list)) {
        std::cerr << "\033[34mSeria error!\033[0m" << std::endl;
    }
    list = Protocol::pack(list);
    MessageCenter::instance().dispatch(_fd, _fd, list);
}

void Connection::Delete_notify(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string from_name = fds[_fd];
    std::string to_name = chat.req().username();
    _redis.deleteHashMember("Friend", from_name, to_name);
}

void Connection::send_user_notify(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    // std::cout << "进入通知" << std::endl;
    std::string username = chat.req().username();
    std::unordered_map<std::string, std::string> result;
    result = _redis.getHash("Friend", username);
    chat::Chat chat_res;
    chat_res.set_action(chat::Actions::USERNTF);
    chat::UserRequest* res = chat_res.mutable_req();
    for(const auto& [key, value] : result) {
        (*res->mutable_result())[key] = value;
    }
    std::string u_res;
    if(!chat_res.SerializeToString(&u_res)) {
        std::cerr << "\033[34mSeria error!\033[0m" << std::endl;
    }
    u_res = Protocol::pack(u_res);
    MessageCenter::instance().dispatch(_fd, _fd, u_res);
}

void Connection::adduser(const std::string& username) {
    // std::lock_guard<std::mutex> lock(_mtx);
    if(users[username] != 0) {
        chat::Chat chat_cancel;
        chat_cancel.set_action(chat::Actions::UNLOGIN);
        std::string cancel;
        chat_cancel.SerializeToString(&cancel);
        cancel = Protocol::pack(cancel);

        MessageCenter::instance().dispatch(_fd, _fd, cancel);
    }

    _user_msg.addUser(_fd, username);

    // 发它的好友消息
    std::vector<std::string> friends;
    friends = _user_msg.getFriend(username);
    chat::Chat chat_list;
    chat_list.set_action(chat::Actions::LOGINLIST);
    chat::FriendLists* lists = chat_list.mutable_friends();
    for(const auto& it : friends) {
        // std::cout << it << std::endl;
        int user = users[it];
        if(user != 0) {
            (*lists->mutable_friends())[it] = true;
        } else {
            (*lists->mutable_friends())[it] = false;
        }
    }

    // 发它的群聊消息
    std::vector<std::string> groups;
    groups = _redis.getGroupList(username);
    chat::GroupList* group_list = chat_list.mutable_group_list();
    for(const auto& it : groups) {
        // std::cout << it << "  hello" << std::endl;
        group_list->add_groups(it);
    }
    std::string lis;
    if(!chat_list.SerializeToString(&lis)) {
        std::cerr << "\033[34mSeria error!\033[0m" << std::endl;
    }
    lis = Protocol::pack(lis);
    MessageCenter::instance().dispatch(_fd, _fd, lis);

    /////// 发一个包让它打印收到的消息
    std::unordered_map<std::string, std::string> result;
    result = _redis.getHash("Response", username);
    chat::Chat chat_res;
    chat_res.set_action(chat::Actions::RESPONSE);
    chat::LoginResponse* response = chat_res.mutable_response();
    for(const auto& [key, value] : result) {
        (*response->mutable_result())[key] = value;
    }
    std::string res;
    if(!chat_res.SerializeToString(&res)) {
        std::cerr << "\033[34mSeria error!\033[0m" << std::endl;
    }
    res = Protocol::pack(res);
    MessageCenter::instance().dispatch(_fd, _fd, res);

    if(!_redis.deleteHash("Response", username)) {
        std::cerr << "\033[34mSet 删除 error!\033[0m" << std::endl;
    }

    heart_time = getNowTime();

}

void Connection::addFriend(const chat::Chat &chat)
{
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string from_user = chat.friend_res().from_name();
    std::string to_user = chat.friend_res().to_name();
    std::string time = Protocol::GetNowTime();
    std::string msg;
    bool decide;
    if(chat.friend_res().decide()) {
        _user_msg.addFriend(from_user, to_user);
        msg = from_user + "\033[36m" + " 同意了您的好友申请！\033[0m";
        decide = true;
    } else {
        msg =  from_user + "\033[31m 拒绝了您的好友申请...\033[0m";
        decide  = false;
    }

    int to_fd = users[to_user];
    if(to_fd == 0) {
        /////// push进去
        std::unordered_map<std::string, std::string> message;
            message = {
                {msg, time}
            };
        _redis.setResKey("Response", to_user, message);
    }

    std::string str;
    str = Seriafdres(to_user, msg, time, decide);
    str = Protocol::pack(str);
    MessageCenter::instance().dispatch(_fd, to_fd, str);
    // std::string res = Seriafdres(_fd, decide, to_user);
}

void Connection::send_addFriend(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::unordered_map<std::string, std::string> message;
    std::string from_name = chat.friend_req().from_username();
    std::string to_name = chat.friend_req().to_username();
    std::string time = chat.friend_req().time();
    if(!_redis.userFieldHexists(to_name, "password")) {
        std::string l = "\033[31m 用户不存在...\033[0m";
        std::string msg = Seriafdres(from_name, l, time, false);
        msg = Protocol::pack(msg);
        MessageCenter::instance().dispatch(_fd, _fd, msg);
    }
    message = {
        {from_name, time}
    };
    int to_fd = users[to_name];
    if(to_fd == 0) {
        std::unordered_map<std::string, std::string> res;   
        std::string a = "\033[1;32m" + from_name + "\033[0m\033[33m请求添加你为好友\033[0m";
        res = {
            {a, time}
        };
        _redis.setResKey("Response", to_name, res);
        _redis.setResKey("Friend", to_name, message);
        return;
    }

    _redis.setResKey("Friend", to_name, message);
    
    std::string msg = Seriafdreq(to_name);
    msg = Protocol::pack(msg);
    MessageCenter::instance().dispatch(_fd, to_fd, msg);
}

void Connection::Send_msg(const chat::Chat& chat) {
    // std::lock_guard<std::mutex> lock(_mtx);
    std::string to_name = chat.chat_fd().to_name();
    std::string from_name = chat.chat_fd().from_name();
    std::string time = chat.chat_fd().time();
    std::string msg = chat.chat_fd().message();
    msg = from_name + ":" + msg;
    std::string user;
    
    if(from_name >to_name) {
        user = from_name +":" + to_name;
        _redis.setChatList("Friend", user, msg);
    } else {
        user = to_name +":" + from_name;
        _redis.setChatList("Friend", user, msg);
    }

    // if(chat.chat_fd().block()) {
    //     return;
    // }

    int to_fd = users[to_name];
    if(to_fd == 0) {
        std::string title = "\033[1;32m您有新的未读消息, 来自:" + from_name + "\033[0m";
        std::unordered_map<std::string, std::string> message;
        std::unordered_map<std::string, std::string> messages;
        messages = {
            {msg, time}
        };
        message = {
            {title, time}
        };
        if(!_redis.userHashHexists("Response", "message", from_name))
        _redis.setResKey("Response", to_name, message);
        std::string user;

        return;
    }

    std::string res = Seriamsg(to_name, msg, time);
    res = Protocol::pack(res);
    
    MessageCenter::instance().dispatch(_fd, to_fd, res);
}

void Connection::Close()
{
    ::close(_fd);
}

std::string Connection::Seriafdreq(const std::string &to_name)
{
    chat::Chat chat;
    chat.set_action(chat::Actions::ADDFDREQ);
    chat::AddFriendRequest *chat_fd = chat.mutable_friend_req();
    auto it = fds.find(_fd);
    std::string from_name;
    if(it != fds.end()) {
        from_name = it->second;
    } else {
        std::cout << "Not find User" << std::endl;
    }
    // 这里可以价格错误包返回;
    chat_fd->set_from_username(from_name);
    chat_fd->set_to_username(to_name);
    std::string time = Protocol::GetNowTime();
    chat_fd->set_time(time);
    return chat.SerializeAsString();
}

std::string Connection::Seriafdres(const std::string &to_name, 
                                   const std::string& msg, 
                                   const std::string& time,
                                   bool decide)
{
    chat::Chat chat;
    chat.set_action(chat::Actions::ADDFDRES);
    chat::AddFriendResponse *chat_fd = chat.mutable_friend_res();
    std::string from_name = fds[_fd];
    chat_fd->set_from_name(from_name);
    chat_fd->set_to_name(to_name);
    chat_fd->set_time(time);
    chat_fd->set_msg(msg);
    chat_fd->set_decide(decide);
    
    return chat.SerializeAsString();
}

std::string Connection::Seriamsg(const std::string& to_name, 
                                 const std::string &msg, 
                                 const std::string& time)
{
    chat::Chat chat;
    chat.set_action(chat::Actions::FRIENDMSG);
    chat::FriendChatMessage *chat_fd = chat.mutable_chat_fd();
    auto it = fds.find(_fd);
    std::string from_name;
    if(it != fds.end()) {
        from_name = it->second;
    } else {
        std::cout << "Not find User" << std::endl;
    }
    chat_fd->set_from_name(from_name);
    chat_fd->set_to_name(to_name);
    chat_fd->set_message(msg);
    chat_fd->set_time(time);
    return chat.SerializeAsString();
}

void Connection::sendMessage(int from_fd, const std::string &msg)
{
    std::lock_guard<std::mutex> lock(_mtx);
    send(_fd, msg.c_str(), msg.size(), 0);
}

bool Connection::checkHeart(const std::string& heart)
{
    // std::lock_guard<std::mutex> lock(t_mtx);
    if(heart != "ping") {
        Close();
        std::cout << "\033[1;31m心跳检测错误\033[0m" << std::endl;
        return false;
    }
    long now_time = getNowTime();
    std::cout << heart_time << std::endl;
    if ((now_time - heart_time) > TIME_OUT)
    {
        Close();
        std::cout << "\033[1;31m心跳检测超时！关闭连接！\033[0m" << std::endl;
        return false;
    }
    else
    {
        heart_time = getNowTime();
        return true;
    }
}

long Connection::getNowTime()
{
    auto now = std::chrono::system_clock::now();

    // duration是"时间段"的意思～
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    return seconds;
}