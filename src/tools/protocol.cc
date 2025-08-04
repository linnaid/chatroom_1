#include "protocol.h"

std::string Protocol::pack(const std::string& payload) {
    uint32_t len = htonl(payload.size());
    // std::cout << len << std::endl;
    std::string msg;
    msg.resize(4);
    memcpy(&msg[0], &len, 4);

    msg += payload;
    return msg;
}

bool Protocol::unpack(std::string& buffer, chat::Chat& chat_msg){
    if(buffer.size() >= 4){
        uint32_t len = 0;
        memcpy(&len, buffer.data(), 4);
        // std::cout << "len:" << len << std::endl;
        len = ntohl(len);
        // std::cout << len << std::endl;
        // std::cout << buffer.size() << std::endl;
        if(buffer.size() < len + 4) {
            // std::cout << "\033[31m长度不够！\033[0m" << std::endl;
            return false;
        }
        // std::cout << buffer.size() << " & " << buffer.c_str() << std::endl;
        std::string msg = buffer.substr(4, len);
        Protocol::Parse_msg(msg, chat_msg);
        buffer.erase(0, 4 + len);
    } else {
        return false;
    }
    return true;
}

void Protocol::Parse_msg(const std::string& msg, chat::Chat& chat_msg) {
    if(!chat_msg.ParseFromString(msg)){
        std::cerr << "\033[31mParse error!\033[0m" << std::endl;
    }
}

// 心跳检测(ping~ pong~)
bool Protocol::is_heartbeat(const std::string& msg){
    return msg == "ping";
}

std::string Protocol::GetNowTime(){
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&now_time);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
    return buf;
}

std::string Protocol::generate_uuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;

    for(int i = 0; i < 32; i++) {
        if(i == 8 || i == 12 || i == 16 || i == 20) {
            ss << "-";
        }
        ss << std::hex << dis(gen);
    }

    return ss.str();
}

