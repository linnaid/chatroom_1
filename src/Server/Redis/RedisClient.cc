
#include <RedisClient.h>
#include <RedisKeys.h>
#include <iostream>

RedisClient::RedisClient(const std::string& redis_uri) : _redis(redis_uri) {};

bool RedisClient::saveUser(const std::string& username,
                           const std::unordered_map<std::string, std::string>& user_info){
    try {
        std::string key = RedisKey::UserKey(username);
        if(_redis.exists(key)) return false;
        if(_redis.type(key) == "hash") return false;
        auto ret = _redis.hset(key, user_info.begin(), user_info.end());
        return ret;
    } catch(const sw::redis::Error &e){
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

bool RedisClient::setResKey(const std::string& username, 
                            const std::string& set_name,
                            const std::unordered_map<std::string, std::string>& msg) {
    try {
        std::string key = RedisKey::RequestKey(username) + ":" +set_name;
        auto ret = _redis.hset(key, msg.begin(), msg.end());
        return ret;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

std::unordered_map<std::string, std::string> RedisClient::getHash(
    const std::string& username, 
    const std::string& set_name) {
        
    std::unordered_map<std::string, std::string> result;

    try {
        std::string key = RedisKey::RequestKey(username) + ":" + set_name;
        _redis.hgetall(key, std::inserter(result, result.begin()));
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
    }

    return result;
}

// 设置验证码
bool RedisClient::setKeyExpire(const std::string& username, 
                               const std::string& value, 
                               int expire_time){
    try {
        _redis.set(username, value, std::chrono::seconds(expire_time));
        return true;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

// 删除元素
bool RedisClient::deleteHashKey(const std::string& username) {
    try {
        std::string key = RedisKey::UserKey(username);
        return _redis.del(key) == 1;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis 删除错误: " << e.what() << std::endl;
        return false;
    }
}

// 删除所有好友申请
bool RedisClient::deleteHashMembers(const std::string& username, 
                                    const std::string& set_name) {
    try {
        std::string key = RedisKey::RequestKey(username) + ":" + set_name;
        return _redis.del(key) == 1;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis 删除错误: " << e.what() << std::endl;
        return false;
    }
}

// 删除好友申请
bool RedisClient::deleteHashMember(const std::string& name,
                             const std::string& set_name, 
                             const std::string& key) {
    try {
        std::string _key = RedisKey::RequestKey(name) + ":" + set_name;
        return _redis.hdel(_key, key) == 1;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis 删除错误: " << e.what() << std::endl;
        return false;
    }
}

// 删除同志消息
bool RedisClient::deleteHash(const std::string& username, 
                            const std::string& set_name) {
    try {
        std::string _key = RedisKey::RequestKey(username) + ":" + set_name;
        return _redis.del(_key);
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis 删除错误: " << e.what() << std::endl;
        return false;
    }
}

// 查找离线消息是否存在；
bool RedisClient::userHashHexists(const std::string& username, 
                                  const std::string& set_name,
                                  const std::string& field) {
    try {
        std::string key = RedisKey::RequestKey(username) + ":" + set_name;
        return _redis.hexists(key, field);
    } catch(const sw::redis::Error &e) {
        return false;
    }
}

// 存储历史消息
bool RedisClient::setChatList(const std::string& username, 
                              const std::string& set_name, 
                              const std::string& field){
    try {
        std::string key = RedisKey::ChatKey(username) + ":" + set_name;
        _redis.rpush(key, field);
        _redis.ltrim(key, -100, -1);
        return true;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 获取历史消息
std::vector<std::string> RedisClient::getChatList(const std::string& username, 
                                                  const std::string& set_name) {
    std::vector<std::string> values;
    try {
        std::string key = RedisKey::ChatKey(username) + ":" + set_name;
        _redis.lrange(key, -100, -1, std::back_inserter(values));
        return values;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return values;
    }                                               
}

// 获取历史消息
std::vector<std::string> RedisClient::getChatOnlineList(const std::string& username, 
                                                  const std::string& set_name) {
    std::vector<std::string> values;
    try {
        std::string key = RedisKey::ChatKey(username) + ":" + set_name;
        _redis.lrange(key, -10, -1, std::back_inserter(values));
        return values;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return values;
    }                                               
}

// 删除历史消息
bool RedisClient::deleteChatList(const std::string& username, 
                                 const std::string& set_name) {
    try {
        std::string key = RedisKey::ChatKey(username) + ":" + set_name;
        return _redis.del(key) == 1;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }    
}

// 储存文件信息
bool RedisClient::setFile(const std::string& username, 
                          const std::string& set_name, 
                          const std::string& field) {
    try {
        std::string key = RedisKey::FileKey(username) + ":" + set_name;
        return _redis.rpush(key, field);
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 删除文件信息 
bool RedisClient::deleteFile(const std::string& username, 
                             const std::string& set_name,
                             const std::string& field) {
    try {
        std::string key = RedisKey::FileKey(username) + ":" + set_name;
        return _redis.lrem(key, 1, field) == 1;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }    
}

// 获取文件信息
std::vector<std::string> RedisClient::getFile(const std::string& username, 
                                              const std::string& set_name) {
    std::vector<std::string> keys;
    auto cursor = 0LL;
    do {
        cursor = _redis.scan(cursor, RedisKey::FileKey(username) + ":*", 100, std::back_inserter(keys));
    } while (cursor != 0);

    std::vector<std::string> values;
    try {
        std::string key = RedisKey::FileKey(username) + ":" + set_name;
        _redis.lrange(key, 0, -1, std::back_inserter(values));
        return values;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return values;
    }  
}

std::unordered_map<std::string, std::vector<std::string>> RedisClient::getAllFile(
    const std::string& username) {
    std::vector<std::string> keys;
    std::unordered_map<std::string, std::vector<std::string>> all_values;
    try {
        auto cursor = 0LL;
        std::string prefix = RedisKey::UserKey("File") + ":" + username;
        do {
            cursor = _redis.scan(cursor, prefix + "*", 100, std::back_inserter(keys));
        } while (cursor != 0);

        for (const auto& key : keys) {
            std::vector<std::string> messages;
            _redis.lrange(key, 0, -1, std::back_inserter(messages));

            std::string friendName = key.substr(key.rfind(':') + 1);
            all_values[friendName] = std::move(messages);
        }
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
    }
    return all_values;
}

// 存储文件名
bool RedisClient::setUserFile(const std::string& username,
                              const std::string& from_name, 
                              const std::string& file_name){
    try {
        std::string key = RedisKey::UserKey("File") + ":" + username + ":" + from_name;
        auto ret = _redis.rpush(key, file_name);
        return ret;
    } catch(const sw::redis::Error &e){
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 删除文件名
bool RedisClient::delUserFile(const std::string& username, 
                                const std::string& to_name) {
    try {
        std::string key = RedisKey::UserKey("File") + ":" + username + ":" + to_name;
        std::cout << key << std::endl;
        auto value = _redis.lpop(key);
        return value.has_value();
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis 删除错误: " << e.what() << std::endl;
        return false;
    }
}

// 时别验证码
std::optional<std::string> RedisClient::userFieldExists(const std::string& username){
    try {
        auto buf =  _redis.get(username);
        if(buf) return *buf;
        else return std::nullopt;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// 判断字段是否存在
bool RedisClient::userFieldHexists(const std::string& username, 
                                   const std::string& field){
    try {
        std::string key = RedisKey::UserKey(username);
        return _redis.hexists(key, field);
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

std::optional<std::string> RedisClient::getUserField(
    const std::string& username, 
    const std::string& field){
    try {
        std::string key = RedisKey::UserKey(username);
        auto val = _redis.hget(key, field);
        if(val) return *val;
        else return nullptr;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return nullptr; 
    }
}


bool RedisClient::setUserGroups(const std::string& username, 
                                const std::string& set_name, 
                                const std::string& element){
    try {
        std::string key = RedisKey::UserKey(username) + ":" + set_name;
        auto ret = _redis.sadd(key, element);
        return ret > 0;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false; 
    }
}

// 移除好友
bool RedisClient::removeUserGroups(const std::string& username, 
                                   const std::string& set_name, 
                                   const std::string& element) {
    try {
        std::string key = RedisKey::UserKey(username) + ":" + set_name;
        auto removed = _redis.srem(key, element);
        return removed > 0;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false; 
    }
}


bool RedisClient::addUserToOnlineLists(const std::string& username){
    try {
        auto ret = _redis.sadd("online_users", username);
        return ret > 0;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

bool RedisClient::removeUserToOnlineLists(const std::string& username){
    try {
        auto ret = _redis.srem("online_users", username);
        return ret > 0;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

// 获取好友
std::vector<std::string> RedisClient::getFields(const std::string& from_name) {
    try {
        std::string key = RedisKey::FriendKey(from_name);
        std::cout << key << std::endl; 
          std::vector<std::string> friends;
        _redis.smembers(key, std::back_inserter(friends));
        return friends;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return {};
    }
}

// 创建群聊
bool RedisClient::addGroup(const std::string& uuid, 
                           const std::string& username,
                           const std::string& groupname,
                           std::unordered_map<std::string, std::string>& g_info) {
    try {
        std::string key = RedisKey::GroupKey(uuid);
        auto ret = _redis.hset(key, g_info.begin(), g_info.end());

        std::string index_key = RedisKey::GroupOwner(username) + ":" + groupname;
        _redis.set(index_key, uuid);

        return true;
    } catch(const sw::redis::Error &e){
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 解散群聊
bool RedisClient::delGroup(const std::string& uuid, 
    const std::string& username, 
    const std::string& g_name) {
    try {
        std::string key = RedisKey::GroupKey(uuid);
        auto ret = _redis.del(key);
        std::string index_key = RedisKey::GroupOwner(username) + ":" + g_name;
        _redis.del(index_key);
        return ret;
    } catch(const sw::redis::Error &e){
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 查看群聊是否存在
bool RedisClient::GroupExists(const std::string& username, 
                               const std::string& g_name) {
    try {
        std::string key = RedisKey::GroupOwner(username) + ":" + g_name;
        if(_redis.exists(key)) std::cout << key << std::endl;
        return _redis.exists(key);
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

// 向群聊加入成员
bool RedisClient::SetGroupMember(const std::string& uuid, 
                                const std::string& username) {
    try {
        std::string key = RedisKey::GroupKey(uuid) + ":" + "members";
        auto ret = _redis.sadd(key, username);
        return ret;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

// 获取群成员
std::vector<std::string> RedisClient::getGroupMember(const std::string& uuid) {
    std::vector<std::string> values;
    try {
        std::string key = RedisKey::GroupKey(uuid) + ":" + "members";
       _redis.smembers(key, std::inserter(values, values.begin()));
        return values;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return values;
    }
}

// 移除群成员
bool RedisClient::removeGroupMember(const std::string& uuid, 
                                    const std::string& username) {
    try {
        std::string key = RedisKey::GroupKey(uuid) + ":" + "members";
        return _redis.srem(key, username);
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

// 移除所有群成员
bool RedisClient::delGroupMember(const std::string& uuid) {
    try {
        std::string key = RedisKey::GroupKey(uuid) + ":" + "members";
        auto ret = _redis.del(key);
        return ret;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 向成员所属群聊添加元素
bool RedisClient::UserSetGroups(const std::string& username, 
                                const std::string& u_name, 
                                const std::string& g_name) {
    try {
        std::string key = RedisKey::UserKey(username) + ":groups";
        std::string name = g_name + "   ---" + u_name;
        auto ret = _redis.sadd(key, name);
        return ret;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

// 向成员所属群聊移除元素
bool RedisClient::UserRemoveGroups(const std::string& username, 
                                   const std::string& g_name, 
                                   const std::string& u_name) {
    try {
        std::string key = RedisKey::UserKey(username) + ":" + "groups";
        std::string name = g_name + "   ---" + u_name;
        return _redis.srem(key, name);
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

// 添加管理人员
bool RedisClient::SetGroupManager(const std::string& uuid, 
                                  std::unordered_map<std::string, std::string>& g_info) {
    try {
        std::string key = RedisKey::GroupKey(uuid) + ":" + "managers";
        auto ret = _redis.hset(key, g_info.begin(), g_info.end());
        return ret;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return false;
    }
}

// 移除管理员
bool RedisClient::removeGroupManager(const std::string& uuid, 
                                    const std::string& username) {
    try {
        std::string key = RedisKey::GroupKey(uuid) + ":" + "managers";
        return _redis.hdel(key, username) == 1;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis 删除错误: " << e.what() << std::endl;
        return false;
    }
}

// 移除所有管理员
bool RedisClient::delGroupManager(const std::string& uuid) {
    try {
        std::string key = RedisKey::GroupKey(uuid) + ":" + "managers";
        auto ret = _redis.del(key);
        return ret;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 获取管理员
std::string RedisClient::getGroupManager(const std::string& uuid, 
                                         const std::string& username) {
    try {
        std::string key = RedisKey::GroupKey(uuid) + ":" + "managers";
        auto val = _redis.hget(key, username);
        return *val;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis 错误: " << e.what() << std::endl;
        return "";
    }
}

// 获取群聊uuid
std::string RedisClient::getGroupuuid(const std::string& username, 
                                      const std::string& g_name) {
    try {
        std::string key = RedisKey::GroupOwner(username) + ":" + g_name;
        auto val = _redis.get(key);
        std::string uuid = *val;
        return uuid;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return "";
    }
}

// 获取群管理员(包括群主)
std::unordered_map<std::string, std::string> RedisClient::getGroupManager(const std::string& uuid) {
    std::unordered_map<std::string, std::string> managers;
    try {
        std::string key = RedisKey::GroupKey(uuid) + ":" + "managers";
        _redis.hgetall(key, std::inserter(managers, managers.begin()));
        return managers;
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error:" << e.what() << std::endl;
        return managers;
    }
}

// 获取成员所属群列表
std::vector<std::string> RedisClient::getGroupList(const std::string& username) {
    std::vector<std::string> values;
    try {
        std::string key = RedisKey::UserKey(username) + ":groups";
        _redis.smembers(key, std::inserter(values, values.begin()));
        return values;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return values;
    }                                               
}


// 设置群成员申请消息
bool RedisClient::setGroupNotify(const std::string& username, 
    const std::unordered_map<std::string, std::string>& g_notify) {
    try {
        std::string key = RedisKey::ManagerKey(username);
        auto ret = _redis.hset(key, g_notify.begin(), g_notify.end());
        return ret;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 获取群成员申请消息
std::unordered_map<std::string, std::string> RedisClient::getGroupNotify (const std::string& username) {
    std::unordered_map<std::string, std::string> notify;
    try {
        std::string key = RedisKey::ManagerKey(username);
        _redis.hgetall(key, std::inserter(notify, notify.begin()));
        return notify;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return notify;
    }
}

// 删除申请消息
bool RedisClient::delGroupNotify(const std::string& username, 
                                 const std::string& name) {
    try {
        std::string key = RedisKey::ManagerKey(username);
        auto ret = _redis.hdel(key, name);
        return ret;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 存储文件名
bool RedisClient::setUserFile(const std::string& uuid,
                              const std::string& from_name, 
                              const std::string& username, 
                              const std::string& file_name){
    try {
        std::string key = RedisKey::UserKey("File") + ":" + uuid + ":" +  username +  ":" + from_name;
        auto ret = _redis.rpush(key, file_name);
        return ret;
    } catch(const sw::redis::Error &e){
        std::cerr << "Redis Error: " << e.what() << std::endl;
        return false;
    }
}

// 删除文件名
bool RedisClient::delUserFile(const std::string& uuid,
                              const std::string& from_name, 
                              const std::string& username, 
                              const std::string& file_name) {
    try {
        std::string key = RedisKey::UserKey("File") + ":" + uuid + ":" + username + ":" + from_name;
        // std::cout << key << std::endl;
        auto value = _redis.lrem(key, 1, file_name);
        return value;
    } catch(const sw::redis::Error &e) {
        std::cerr << "Redis 删除错误: " << e.what() << std::endl;
        return false;
    }
}

// 获取文件名
std::unordered_map<std::string, std::vector<std::string>> RedisClient::getAllFile(
    const std::string& uuid, 
    const std::string& username) {
    std::vector<std::string> keys;
    std::unordered_map<std::string, std::vector<std::string>> all_values;
    try {
        auto cursor = 0LL;
        std::string prefix = RedisKey::UserKey("File") + ":" + uuid + ":" + username;
        do {
            cursor = _redis.scan(cursor, prefix + "*", 100, std::back_inserter(keys));
        } while (cursor != 0);

        for (const auto& key : keys) {
            std::vector<std::string> messages;
            _redis.lrange(key, 0, -1, std::back_inserter(messages));

            std::string groupfriendName = key.substr(key.rfind(':') + 1);
            all_values[groupfriendName] = std::move(messages);
        }
    } catch (const sw::redis::Error &e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
    }
    return all_values;
}





