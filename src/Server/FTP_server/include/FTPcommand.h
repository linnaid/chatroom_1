#pragma once

#include "include.h"
#include "FTPconnection.h"
// #include "json.hpp"

class FTPconnect;

class command{
public:
    void cmd_execute(FTPconnect& conn, const std::string cmd_line);
    
private:
    static void STOR(FTPconnect& conn, const std::string& path, const std::string& username, const std::string& to_name);
    static void RETR(FTPconnect& conn, const std::string& path, const std::string& from_name, const std::string& to_name);
    static void LIST(FTPconnect& conn, const std::string& path);
    static void QUIT(FTPconnect& conn);
};