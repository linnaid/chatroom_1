#pragma once

#include "include.h"
#include "user_page.h"
#include "user_chat.pb.h"

class Protocol {
public:
    static std::string pack(const std::string& payload);
    
    static bool unpack(std::string& buffer, chat::Chat& chat_msg);

    static bool is_heartbeat(const std::string& msg);

    static std::string GetNowTime();

    static std::string generate_uuid();

    static void Parse_msg(const std::string& msg, chat::Chat& chat_msg);

};
