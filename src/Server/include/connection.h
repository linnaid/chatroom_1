#pragma once

#include "include.h"
#include "protocol.h"
#include "user_chat.pb.h"
#include "User_manager.h"
#include "user_page.h"
#include "messageCenter.h"
#include "RedisClient.h"

#define TIME_OUT 40

class Connection {
public:
    Connection(int fd, UserManager& user_msg);

    bool readMessage();

    void sendMessage(int from_fd, const std::string& msg);

    void Close();

    const std::string& get_username() const;

    bool checkHeart(const std::string& heart);

private:
    long getNowTime();
    std::string Seriamsg(const std::string& to_name, const std::string& msg, const std::string& time);
    std::string Seriafdreq(const std::string& to_name);
    std::string Seriafdres(const std::string& to_name, 
                           const std::string& msg, 
                           const std::string& time,
                           bool decide);
    // 以下对好友
    void addFriend(const chat::Chat& chat);
    void send_addFriend(const chat::Chat& chat);
    void send_user_notify(const chat::Chat& chat);
    void adduser(const std::string& username);
    void Delete_notify(const chat::Chat& chat);
    void send_List(const chat::Chat& chat);
    void Send_msg(const chat::Chat& chat);
    void Send_delete_friend(const chat::Chat& chat);
    void send_offline_msg(const chat::Chat& chat);
    void send_online_msg(const chat::Chat& chat);
    void block(const chat::Chat& chat);
    void send_file(const chat::Chat& chat);
    void recv_file(const chat::Chat& chat);
    void send_file_req(const chat::Chat& chat);
    void unblock(const chat::Chat& chat);

    // 以下对群聊
    void add_group(const chat::Chat& chat);
    void join_group(const chat::Chat& chat);
    void group_list(const chat::Chat& chat);
    void group_notify(const chat::Chat& chat);
    void group_decide(const chat::Chat& chat);
    void group_choice(const chat::Chat& chat);
    void group_members(const chat::Chat& chat);
    void group_chat(const chat::Chat& chat);
    void group_chat_list(const chat::Chat& chat);
    void group_online_chat_list(const chat::Chat& chat);
    void leave_group(const chat::Chat& chat);
    void add_member(const chat::Chat& chat);
    void del_member(const chat::Chat& chat);
    void disband_froup(const std::string& g_name, const std::string& u_name);
    void add_manager(const chat::Chat& chat);
    void del_manager(const chat::Chat& chat);
    void send_file_group(const chat::Chat& chat);
    void recv_file_group(const chat::Chat& chat);
    void send_file_req_group(const chat::Chat& chat);

    // 以下是注销账号的相关函数
    void del_account(const chat::Chat& chat);
    


    int _fd;
    std::mutex _mtx;
    std::mutex t_mtx;
    std::string _username;
    std::string _buffer;
    UserManager& _user_msg;
    long heart_time;
    RedisClient _redis;
};