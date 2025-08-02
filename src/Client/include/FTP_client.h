#pragma once

#include "protocol.h"
#include "user_chat.pb.h"
#include "include.h"


class FTPClient
{
public:
    FTPClient(const std::string& decide, const std::string& from_name, const std::string& to_name, const std::string& path, const std::string& file_name);
    void init();
    // 发送文件；
    void sendFile();
    // 接收文件；
    void getFile();
    // 查看目录
    void listFile();
    ~FTPClient();
private:
    bool Send(int sockfd, const std::string& msg);
    void Connect(int sockfd, sockaddr_in cli);//
    std::string get_directory_path();
    bool Recive();
    void Decide(bool b);
    void openPassiveMode(int a);
    void List();
    void Stor();
    void Retr();
    // void Quit();
    std::ofstream File_creat(char* buf);

    std::string _decide;
    std::string file_path;
    std::string _file_name;
    std::string _from_name;
    std::string _to_name;

    int _sockfd;
    int link_sock;
    sockaddr_in _cli;
    int _port;

    std::string _ip;
    
    int p1, p2;
    std::string passive_ip();
    int n = 0;
};