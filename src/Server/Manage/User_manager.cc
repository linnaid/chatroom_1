#include "User_manager.h"

std::unordered_map<std::string, int> users;
std::unordered_map<int, std::string> fds;

UserManager::UserManager()
: _redis("tcp://127.0.0.1:6379")
{}

// 登入用户
void UserManager::addUser(int fd, const std::string& username) {
    std::lock_guard<std::mutex> lock(_mtx);
    _redis.addUserToOnlineLists(username);
    std::cout << "\033[33m" << fd << "---" << username <<  "\033[0m" << std::endl;
    users[username] = fd;
    fds[fd] = username;
}

// 登出/注销 用户
void UserManager::removeUser(int cli) {
    std::lock_guard<std::mutex> lock(_mtx);
    std::string username = fds[cli];
    _redis.removeUserToOnlineLists(username);
    users.erase(username);
    fds.erase(cli);
}

bool UserManager::addFriend(const std::string& from_name, 
                            const std::string& to_name) {
    std::lock_guard<std::mutex> lock(_mtx);
    _redis.setUserGroups("Friend", from_name, to_name);
    _redis.setUserGroups("Friend", to_name, from_name);
    return true;
}

bool UserManager::deleteFriend(const std::string& from_name, 
                               const std::string& to_name) {
    std::lock_guard<std::mutex> lock(_mtx);
    _redis.removeUserGroups("Friend", from_name, to_name);
    _redis.removeUserGroups("Friend", to_name, from_name);
    return true;

}

std::vector<std::string> UserManager::getFriend(const std::string& from_name) {
    std::lock_guard<std::mutex> lock(_mtx);
    return _redis.getFields(from_name);
}

std::string UserManager::getGroup_uuid(const std::string& u_name, const std::string& g_name) {
    std::lock_guard<std::mutex> lock(_mtx);
    return _redis.getGroupuuid(u_name, g_name);
}