#pragma once

#include "include.h"
#include "RedisClient.h"
#include "user_page.h"

extern std::unordered_map<std::string, int> users;
extern std::unordered_map<int, std::string> fds;

class UserManager {
public:
    UserManager();
    
    void addUser(int fd, const std::string& username);

    bool addFriend(const std::string& from_name, const std::string& to_name);

    bool deleteFriend(const std::string& from_name, const std::string& to_name);

    void removeUser(int cli);

    std::vector<std::string> getFriend(const std::string& from_name);

    std::string getGroup_uuid(const std::string& u_name, const std::string& g_name);

private:

    RedisClient _redis;
    std::mutex _mtx;
};