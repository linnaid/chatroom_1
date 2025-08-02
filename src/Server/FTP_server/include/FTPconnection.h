#pragma once

#include "include.h"
// #include "json.hpp"
#include "FTPcommand.h"
#include "Dataconnect.h"

struct User{
    std::string username;
    std::string password;
};

class command;
class Dataconnect;


class FTPconnect{
public:
    FTPconnect(int cli_fd);
    ~FTPconnect()=default;
    void process_input(const char* buf, int len);
    // bool is_login_in(const std::string& name, const std::string& pass);
    void send_response(std::string test) const;
    void Close();
    // void set_username(const std::string& name);
    // const std::string& get_username();
    bool set_pasv_conn();
    std::string& get_current_path();

    // bool login;
    Dataconnect* data_conn;
    
private:
    void handle_command(const std::string& cmd_line);
    // std::vector<User> LoadUsers(const std::string path);

    int control_fd;
    bool _pasv;
    std::string current_dir;
    std::map<int, std::shared_ptr<command>> comds;
    // std::shared_ptr<command> comd;
    std::vector<std::string> cmd_lines;
    std::vector<User> users;
    User user;
    std::mutex user_mutex;
    std::mutex comd_mutex;
};
