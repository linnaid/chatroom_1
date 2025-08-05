#pragma once

#include <unordered_map>
#include <sw/redis++/redis++.h>
#include <string>

class RedisClient{
public:
    explicit RedisClient(const std::string& redis_uri);

    // 以下对好友，包括文件
    bool saveUser(const std::string& username,
                  const std::unordered_map<std::string, std::string>& user_info);
    
    bool setKeyExpire(const std::string& key, const std::string& value, int expire_time);

    bool userFieldHexists(const std::string& username, const std::string& field);

    std::optional<std::string> userFieldExists(const std::string& username);

    std::optional<std::string> getUserField(const std::string& username, const std::string& field);

    bool setUserGroups(const std::string& username, const std::string& set_name, const std::string& element);

    bool removeUserGroups(const std::string& username, const std::string& set_name, const std::string& element);
    
    bool addUserToOnlineLists(const std::string& username);

    bool removeUserToOnlineLists(const std::string& username);

    std::vector<std::string> getFields(const std::string& from_name);

    bool setResKey(const std::string& username, const std::string& set_name, const std::unordered_map<std::string, std::string>& msg);

    bool deleteHash(const std::string& username, const std::string& set_name);

    bool deleteHashKey(const std::string& key);

    bool userHashHexists(const std::string& username, const std::string& set_name, const std::string& field);

    bool deleteHashMember(const std::string& name, const std::string& set_name, const std::string& key);

    bool deleteHashMembers(const std::string& username, 
                                    const std::string& set_name);

    bool setChatList(const std::string& username, const std::string& set_name, const std::string& field);

    bool deleteChatList(const std::string& username, const std::string& set_name);

    std::vector<std::string> getChatList(const std::string& username, const std::string& set_name);
    
    std::vector<std::string> getChatOnlineList(const std::string& username, const std::string& set_name);

    std::unordered_map<std::string, std::string> getHash(const std::string& username, const std::string& set_name);

    bool setFile(const std::string& username, const std::string& set_name, const std::string& field);

    bool deleteFile(const std::string& username, const std::string& set_name, const std::string& field);

    std::vector<std::string> getFile(const std::string& username, const std::string& set_name);
    
    std::unordered_map<std::string, std::vector<std::string>> getAllFile(const std::string& username);

    bool setUserFile(const std::string& username, const std::string& from_name, const std::string& file_name);
    
    bool delUserFile(const std::string& username, const std::string& to_name);

    // 以下对群聊
    bool addGroup(const std::string& uuid, const std::string& username, const std::string& groupname,
                  std::unordered_map<std::string, std::string>& g_info);

    bool delGroup(const std::string& uuid, const std::string& username, const std::string& g_name);

    bool GroupExists(const std::string& username, const std::string& g_name);

    bool SetGroupMember(const std::string& uuid, const std::string& username);
    
    std::vector<std::string> getGroupMember(const std::string& uuid);

    bool UserSetGroups(const std::string& username, const std::string& u_name, const std::string& g_name);

    bool SetGroupManager(const std::string& uuid, std::unordered_map<std::string, std::string>& g_info);

    std::string getGroupuuid(const std::string& username, const std::string& g_name);

    std::unordered_map<std::string, std::string> getGroupManager(const std::string& uuid);

    std::vector<std::string> getGroupList(const std::string& username);

    std::unordered_map<std::string, std::string> getGroupNotify (const std::string& username);

    bool setGroupNotify(const std::string& username, const std::unordered_map<std::string, std::string>& g_notify);

    bool delGroupNotify(const std::string& username, const std::string& g_name);

    bool removeGroupMember(const std::string& uuid, const std::string& username);
    
    bool removeGroupManager(const std::string& uuid, const std::string& username);
    
    std::string getGroupManager(const std::string& uuid, const std::string& username);
    
    bool UserRemoveGroups(const std::string& username, const std::string& g_name, const std::string& u_name);

    bool delGroupManager(const std::string& uuid);

    bool delGroupMember(const std::string& uuid);

    bool setUserFile(const std::string& uuid,
                    const std::string& from_name, 
                    const std::string& username, 
                    const std::string& file_name);

    bool delUserFile(const std::string& uuid,
                    const std::string& from_name, 
                    const std::string& username, 
                    const std::string& file_name);

    std::unordered_map<std::string, std::vector<std::string>> getAllFile(
    const std::string& uuid, 
    const std::string& username);


    private:
    sw::redis::Redis _redis;
};

