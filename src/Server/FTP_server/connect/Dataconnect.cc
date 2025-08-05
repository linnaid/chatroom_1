#include "Dataconnect.h"

Dataconnect::Dataconnect()
: _redis("tcp://127.0.0.1:6379")
{}

void Dataconnect::send_passive(const int& cli_fd, FTPconnect& conn)
{
    int p1, p2;
    listen_pasv = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in sock_pasv;
    sock_pasv.sin_family = AF_INET;
    sock_pasv.sin_port = 0; 
    bind(listen_pasv, (sockaddr*)&sock_pasv, sizeof(sock_pasv));
    listen(listen_pasv, SOMAXCONN);
    if(make_nonblocking(listen_pasv) == -1){
        perror("LISTEN_PASV MAKE_NONBLOCK ERROR");
        return;
    }
    sockaddr_in artual_port;
    socklen_t len1 = sizeof(artual_port);
    getsockname(listen_pasv, (sockaddr*)&artual_port, &len1);
    sockaddr_in local_ip;
    socklen_t len2 = sizeof(local_ip);
    getsockname(cli_fd, (sockaddr*)&local_ip, &len2);
    char buf[1024];
    inet_ntop(AF_INET, &local_ip.sin_addr.s_addr, buf, sizeof(buf));
    int ip1, ip2, ip3, ip4;
    char dot;
    std::istringstream iss(buf);
    iss >> ip1 >> dot >> ip2 >> dot >> ip3 >> dot >> ip4;
    uint16_t _port = ntohs(artual_port.sin_port);
    p1 = _port / 256;
    p2 = _port % 256;
    std::ostringstream oss;
    oss << "227 Entering Passive Mode (" << ip1 << "," << ip2 << "," << ip3 << "," << ip4 << "," << p1 << "," << p2 << ")";
    std::string ass = oss.str();
    conn.send_response(ass);
}

void Dataconnect::accept_data()
{
    while(1){
        data_fd = accept(listen_pasv, nullptr, nullptr);
        if(data_fd < 0){
            if(data_fd == EAGAIN || data_fd == EWOULDBLOCK)
            continue;
            // else
            // perror("ACCEPT_DATA ERROR");
        }
        else
        break;
    }
    std::cout << "PASV Openning" << std::endl;
    if(make_nonblocking(data_fd) == -1){
        perror("DATA_FD MAKE_NONBLOCK ERROR");
        return;
    }
}

std::vector<std::string> Dataconnect::get_file_name(const std::string& username, const std::string& to_name) {    
    std::vector<std::string> file_name = _redis.getFile(username, to_name);
    
    return file_name;
}


void Dataconnect::send_file(const std::string& path, 
                            FTPconnect& conn, 
                            const std::string& from_name, 
                            const std::string& to_name)
{
    // std::cout << "111" << std::endl;
    std::vector<std::string> file_names = get_file_name(to_name, from_name);
    
    std::string file_name = file_names[0];
    std::cout << file_name << std::endl;
    std::ifstream file(file_name, std::ios::binary);
    if(!file){
        perror("OPEN_FILE ERROR");
        conn.send_response("550 File not found.");
        return;
    }
    char test[1024];
    conn.send_response("150 Opening BINARY mode data connection...");
    while(file)
    {
        std::memset(test, 0, sizeof(test));
        file.read(test, sizeof(test));
        // std::cout << test << std::endl;
        std::streamsize size = file.gcount();
        ssize_t total_sent = 0;
        while(total_sent < size)
        {
            // std::cout << "len error" << std::endl;
            ssize_t len = send(data_fd, test + total_sent, size - total_sent, 0);
            // std::cout << "len = " << len << std::endl;
            if(len < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
                // perror("SEND ERROR");
                break;
            }
            total_sent += len;
        }
    }
    // file.close();
    Close();
    conn.send_response("226 Transfer complete.");
    if(from_name.find(' ') == std::string::npos) {
        _redis.deleteFile(to_name, from_name, file_name);   
    }
}

void Dataconnect::recive_file(const std::string& path, 
                              FTPconnect& conn, 
                              const std::string& from_name, 
                              const std::string& to_name)
{
    std::ofstream f_open = file_creat(path, from_name, to_name);
    if(!f_open){
        perror("FILE_CREAT ERROR");
        conn.send_response("550 Requested action not taken. File unavailable");
        Close();
        return;
    }
    conn.send_response("150 Opening BINARY mode data connection for filename.txt.");
    char buf[1024];
    while(1)
    {
        ssize_t l = recv(data_fd, buf, sizeof(buf), 0);
        if(l > 0)  f_open.write(buf, l);
        else if(l == 0) {
            std::cout << "Client closed connection." << std::endl;
            break;
        }
        else {
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                continue;
            }
            else{
                perror("RECV_FILE ERROR");
                break;
            }
            
        }
    }
    Close();
    f_open.close();
    conn.send_response("226 Transfer complete");
}

int Dataconnect::make_nonblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    if(flag == -1) return -1;
    return fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void Dataconnect::Close()
{
    close(data_fd);
    close(listen_pasv);
}

std::ofstream Dataconnect::file_creat(const std::string& path, const std::string& username, const std::string& to_name)
{
    (void)path;
    std::string f_path = "/home/linnaid/ChatRoom/chatroom_1/SendFile";
    std::string time = getNowTime();
    std::string f = getRandom();
    std::string file_name = time + "_" + username + "_" + f + ".dat";
    std::string p = f_path + "/" + file_name;
    _redis.setFile(username, to_name, p);
    std::filesystem::path fdname = std::filesystem::path(f_path) / file_name;
    std::ofstream f_open(fdname, std::ios::binary);
    return f_open;
}

std::string Dataconnect::getRandom() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(0, 99999);
    int random_num = dist(gen);
    std::stringstream num;
    num << std::setw(5) << std::setfill('0') << random_num;
    return num.str();
}

void Dataconnect::send_data(const std::string& _data)
{
    if(send(data_fd, _data.c_str(), _data.size(), 0) < 0){
        perror("SEND_DATA ERROR");
    }
    Close();
}

std::string Dataconnect::getNowTime()
{
    auto now = std::chrono::system_clock::now();

    // duration是"时间段"的意思～
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    std::string time = std::to_string(seconds);
    return time;
}