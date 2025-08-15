#include "User_auth_c.h"
#include "User_chat.h"
#include "Heart.h"


int main(int argc, char** argv){
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    std::thread t[2];
    Heart cli_heart;
    t[1] = std::thread(&Heart::init, &cli_heart);
    UserAuth_c cli_auth(argc, argv);
    cli_auth.init();
    UserChat cli_chat;
    
    t[0] = std::thread(&UserChat::init, &cli_chat);    

    t[0].join();
    running = false;
    _notify.notify_all();
    t[1].join();
}