#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <curl/curl.h>
#include <sstream>
#include <iostream>

// smtp_url-邮件发送服务器的地址;
class MailSender{
public:
    MailSender(const std::string& smtp_url,
               const std::string& username,
               const std::string& password);

    bool sendMail(const std::string& from,
                  const std::vector<std::string>& to,
                  const std::string& subject,
                  const std::string& content);
private:
    std::string smtp_url_;
    std::string username_;
    std::string password_;
};





