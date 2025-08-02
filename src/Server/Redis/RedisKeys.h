
#pragma once

#include <string>

namespace RedisKey{
    inline std::string UserKey(const std::string& username){
        return "user:" + username;
    }

    inline std::string GroupKey(const std::string& groupname){
        return "group:" + groupname;
    }

    inline std::string GroupOwner(const std::string& username) {
        return  "group_owner:" + username;
    }

    inline std::string FriendKey(const std::string& username) {
        return "user:Friend:" + username;
    }
    
    inline std::string RequestKey(const std::string& username) {
        return "requeset:" + username;
    }

    inline std::string ChatKey(const std::string& username) {
        return "chat:" + username;
    }

    inline std::string FileKey(const std::string& username) {
        return "file:" + username;
    }

    inline std::string ManagerKey(const std::string& username) {
        return "manager:" + username;
    }

}
