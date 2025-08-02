#include "User_auth_s.h"
#include "MasterServer.h"
#include "masterreactor.h"

// #define PORT1 8080
// #define PORT2 9090

int main(){
    std::thread t[3];
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    UserAuth_s auth(PORT1);
    Master chat;
    master file(PORT3);
    t[0] = std::thread(&UserAuth_s::init, &auth);
    t[1] = std::thread(&Master::init, &chat);
    t[2] = std::thread(&master::init, &file);
    t[0].join();
    t[1].join();
    t[2].join();
    // auth.init();
    // auth.run();
}