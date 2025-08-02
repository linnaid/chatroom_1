#pragma once

#include "include.h"
#include "user_chat.pb.h"
#include "User_auth_c.h"
#include "protocol.h"
#include "user_page.h"
#include "Clear.hpp"
#include "FTP_client.h"

enum class Select {
    FRIEND = 1,
    GROUP = 2,

    QUIT = 0
};

enum class Actions {
    ADDFRIEND = 1,
    CHATFD = 2,
    FDLIST = 3,
    DELETEFRIEND = 4,
    FINDREQ = 5,
    BLOCKFRIEND = 6,
    UNBLOCKFRIEND = 10,
    ADDFRIENDRES = 8,
    MSG = 7,
    QUITNTY = 9,
    SENDFILE = 11,
    RECVFILE = 13,
    LOOKFILE = 12,
    QUIT = 0
};

enum class Group {
    ADDGROUP = 1,
    JOINGROUP = 2,
    MANAGEGROUP = 3,
    GROUPLIST = 4,
    FINDREQ = 5,
    DECIDEREQ = 6,
    QUITNTY = 9,
    QUIT = 0
};

enum class ManageGroups {
    GROUPMEMBERS = 1,
    CHATMSG = 2,
    CHATFILE = 3,
    LOOKFILE = 4,
    LEAVEGROUP = 5,
    MSG = 6,
    RECVFILEGROUP = 7,

    DECIDEADDGROUP = 8,
    DECIDEDELGROUP = 9,

    ADDADMINISTRATOR = 10,
    DELADMINISTRATOR = 11,
    DISBANDGROUP = 12,
    QUITNTY = 13,
    QUIT = 0
};

class friend_Page;
class Clear;

class UserChat {
public:
    UserChat();
    ~UserChat();
    void init();
    

private:
    void runSend();

    void runRecive();

    void runFriend();

    void runGroup();

    void runGroups();

    void main_run();

    void send_login();

    bool Send(const std::string& buf);


    // 以下是对好友的函数处理：
    void send_add_friend();

    void recv_add_friend(const chat::Chat& chat_msg);

    void print_add_friend(const chat::Chat& chat_msg);

    void send_delete_friend();

    void send_offline();

    void block_friend();

    void unblock_friend();

    void send_file();

    void recive_file();
    
    void look_file();

    void decide_add_friend();

    void print_friends(const chat::Chat& chat_msg);

    void print_delete(const chat::Chat& chat_msg);

    void printf_delete_req(const chat::Chat& chat_msg);

    void print_response(const chat::Chat& chat_msg);

    void print_msg(const chat::Chat& chat_msg);

    void print_offline_msg(const chat::Chat& chat_msg);

    void print_cancel();

    void print_block(const std::string& chat_msg);

    void print_unblock(const std::string& chat_msg);
    
    void print_send_file(const chat::Chat& chat_msg);
    
    void print_look_file(const chat::Chat& chat_msg);    

    void save_friends(const chat::Chat& chat_msg);

    void chat_friend();

    void send_msg(const std::string& to_name);

    void send_List();

    void recive_msg(const std::string& to_name);

    void print_notify(const chat::Chat& chat_msg);

    void joinThread(std::thread& t);

    void handleReq();

    // 这个函数时对处理结果的发送
    void decide_add_friend(const std::string& to_name, int select);
    void delete_add_friend(const std::string& name);

    // 以下是对群函数的处理
    void send_add_group();
    void join_group();
    void group_list();
    void group_request();
    void deicde_group_request();
    void choice_groups();

    void disband_group();
    void leave_group();
    void send_group_member();
    void chat_group();
    void send_file_group();
    void recive_file_group();
    void look_file_group();
    void send_group_msg();
    void recv_group_msg();
    void chat_group_list();
    void add_group_member();
    void del_group_member();
    void add_group_manager();
    void del_group_manager();


    void recv_add_group(const chat::Chat& chat_msg);
    void print_join_group(const chat::Chat& chat_msg);
    void decide_join_group(const chat::Chat& chat_msg);
    void print_group_list(const chat::Chat& chat_msg);
    void print_group_notify(const chat::Chat& chat_msg);
    void recv_choice_groups(const chat::Chat& chat_msg);
    void print_group_members(const chat::Chat& chat_msg);
    void print_group_msg(const chat::Chat& chat_msg);
    void print_chat_group_list(const chat::Chat& chat_msg);
    void print_leave_group(const chat::Chat& chat_msg);
    void print_add_member(const chat::Chat& chat_msg);
    void print_add_res(const chat::Chat& chat_msg);
    void print_del_member(const chat::Chat& chat_msg);
    void print_del_res(const chat::Chat& chat_msg);
    void print_disband_group(const chat::Chat& chat_msg);
    void print_add_manager(const chat::Chat& chat_msg);
    void print_add_manager_res(const chat::Chat& chat_msg);
    void print_del_manager(const chat::Chat& chat_msg);
    void print_del_manager_res(const chat::Chat& chat_msg);
    void print_send_group_file(const chat::Chat& chat_msg);
    void print_look_group_file(const chat::Chat& chat_msg);





    int _sockfd;
    std::atomic<bool> _running;
    std::atomic<bool> _running_recv;
    std::atomic<bool> _run;
    std::atomic<bool> _main_run;
    std::atomic<bool> group_exixt;
    std::thread t1;
    std::thread t2;
    std::thread main_t1;
    std::thread main_t2;
    int choice;
    Actions action;
    Group group;
    ManageGroups groups;
    Select select;
    std::string _ip;
    std::string msg;
    int _port;
    sockaddr_in _fd;
    std::vector<std::string> friend_res;
    std::unordered_set<std::string> friends;
    // std::unordered_map<std::string, bool> friedns_online;
    std::unordered_set<std::string> friends_block;
    std::unordered_set<std::string> friends_by_block;
    std::unordered_map<std::string, std::vector<std::string>> friends_files;
    std::unordered_map<std::string, std::vector<std::string>> group_files;
    std::unordered_set<std::string> user_groups;
    std::unordered_set<std::string> user_managers;
    std::string user_name;
    std::string group_name;
    std::string _name;
    std::string _statue;

};