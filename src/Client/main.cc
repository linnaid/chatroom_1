#include "User_auth_c.h"
#include "User_chat.h"
#include "Heart.h"


int main(int argc, char** argv){
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    UserAuth_c cli_auth(argc, argv);
    cli_auth.init();
    UserChat cli_chat;
    Heart cli_heart;
    std::thread t[2];
    
    t[0] = std::thread(&UserChat::init, &cli_chat);
    sleep(1);
    t[1] = std::thread(&Heart::init, &cli_heart);

    t[0].join();
    running = false;
    t[1].join();
}