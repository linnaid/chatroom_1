#pragma once

#include "include.h"
#include "FTPconnection.h"
#include "RedisClient.h"

class FTPconnect;

class Dataconnect{
public:
    Dataconnect();
    void send_passive(const int& cli_fd, FTPconnect& conn);
    void accept_data();
    void send_file(const std::string& path, FTPconnect& conn, const std::string& from_name, const std::string& to_name);
    void recive_file(const std::string& path, FTPconnect& conn, const std::string& from_name, const std::string& to_name);
    void Close();  
    void send_data(const std::string& _data);
    
private:
    std::vector<std::string> get_file_name(const std::string& username, const std::string& to_name);
    int make_nonblocking(int fd); 
    std::string getNowTime();
    std::string getRandom();
    std::ofstream file_creat(const std::string& path, const std::string& username, const std::string& to_name);
    
    RedisClient _redis;
    int listen_pasv;
    int data_fd;
};
