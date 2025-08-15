#include "FTP_client.h"


int t = -1;
// std::vector<char> buf(MAX_NUM);
std::vector<char> arr(MAX_NUM);
std::vector<char> ass(MAX_NUM);


FTPClient::FTPClient(const std::string& decide, 
                     const std::string& from_name, 
                     const std::string& to_name, 
                     const std::string& path,
                     const std::string& file_name)
        :_decide(decide),
        _from_name(from_name),
        _to_name(to_name),
        file_path(path),
        _file_name(file_name),
        _sockfd(t),
        link_sock(t),
        _port(PORT3),
        _ip(IP),
        p1(0),
        p2(0)
        {}

void FTPClient::init()
{
    
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(_sockfd < 0)
    {
        perror("Sockfd Error");
        exit(1);
    }
    bzero(&_cli, sizeof(_cli));
    _cli.sin_family = AF_INET;
    _cli.sin_port = htons(_port);
    inet_pton(AF_INET, _ip.c_str(), &_cli.sin_addr.s_addr);

    if(_decide == "UPLOAD") {
        sendFile();
    } else if (_decide == "DOWNLOAD") {
        getFile();
    }

}

FTPClient::~FTPClient()
{
    if(_sockfd >= 0)
    {
        close(_sockfd);
    }
    if(link_sock >= 0)
    {
        close(link_sock);
    }
}


int _send = 0;
void FTPClient::sendFile()
{
    // std::cout << "jjj" << std::endl;
    // sleep(3);
    // std::cout << "hello" << std::endl;
    Connect(_sockfd, _cli);
    Send(_sockfd, "PASV\r\n");
    if(!Recive()) {
        std::cout << "\033[31m接收失败,PASV模式无法开启...\033[0m" << std::endl;
    }

    int a = strlen(arr.data());
    openPassiveMode(a); // PASV
    std::string msg = "STOR " + file_path + " " + _from_name + " " + _to_name + "\r\n";
    Send(_sockfd, msg);
    // std::cout << msg << std::endl;
    Recive();
    if(strncmp(arr.data(), "150 Opening BINARY mode data connection for filename.txt.", 44) == 0)
    {
        // std::cout << arr.data() <<std::endl;

        // 发送文件
        Stor();

        if(Recive())
        {
            if(strncmp(arr.data(), "226 Transfer complete", 16) == 0)
            {
                // close(link_sock);
                std::cout << "\033[1;32m文件已上传\033[0m" << std::endl;
            }
        }
    }

    Send(_sockfd, "QUIT\r\n");

}

void FTPClient::getFile() {

    Connect(_sockfd, _cli);
    Send(_sockfd, "PASV\r\n");
    if(!Recive()) {
        std::cout << "\033[31m接收失败,PASV模式无法开启...\033[0m" << std::endl;
    }

    int a = strlen(arr.data());
    openPassiveMode(a); // PASV

    std::string msg = "RETR " + file_path + " " + _from_name + " " + _to_name + "\r\n";
    Send(_sockfd, msg);
    std::cout << msg << std::endl;
    Recive();
    if(strncmp(arr.data(), "150 Opening BINARY mode data connection...", 42) == 0)
    {
        // 接收文件
        Retr();
        if(Recive())
        {
            if(strncmp(arr.data(), "226 Transfer complete.", 16) == 0)
            {
                std::cout << "\033[1;32m文件已下载\033[0m" <<std::endl;
                _send = 0;
            }
        }
    }
    Send(_sockfd, "QUIT\r\n");
}

std::string FTPClient::get_directory_path() {
    char path[MAX_NUM];
    ssize_t len = readlink("/proc/self/exe", path, MAX_NUM);
    std::filesystem::path _path = std::string(path, (len > 0) ? len : 0);
    std::string real_path = _path.parent_path().parent_path();
    real_path = real_path + "/File";
    return real_path;
}

std::ofstream FTPClient::File_creat(char* file_path)
{
    // std::cout << fd << std::endl;
    std::filesystem::path fdpath(file_path);
    // 获取目录
    std::string time = Protocol::GetNowTime();
    std::string chage_file = time + "_" + _to_name + "_" + _file_name;
    std::string path_ = get_directory_path();
    std::filesystem::path f_path = std::filesystem::path(path_) / chage_file;
    std::ofstream open_file(f_path, std::ios::binary);
    if(!open_file)
    {
        perror("OPEN_file Error");
    }
    return open_file;
}

void FTPClient::Retr()
{
    std::ofstream file = File_creat(file_path.data());
    if(!file)
    {
        perror("OPEN_FILE ERROR");
        close(link_sock);
    }
    std::cout << "\033[34m正在接收文件...\033[0m" <<std::endl;
    std::cout << _file_name << std::endl;
    char re[MAX_NUM];
    while(1)
    {
        ssize_t k = recv(link_sock, re, sizeof(re), 0);
        if(k < 0)
        {
            perror("RETR_RECIVE ERROR");
            break;
        }
        else if(k == 0)
        {
            // std::cout << "Server closed connection." << std::endl;
            break;
        }
        // std::cout << re << std::endl;
        else file.write(re, k);
    }
    // std::cout << "lll" << std::endl;
    close(link_sock);
}

void FTPClient::Stor()
{

    std::ifstream file(file_path, std::ios::binary);
    if(!file)
    {
        perror("Failed to open file");
        n = -1;
        return;
    }
    ass.clear();
    ass.resize(1024);
    std::cout << "\033[34m正在发送文件...\033[0m" <<std::endl;
    while(file)
    {
        file.read(ass.data(), ass.size());
        // std::cout << ass.data() << std::endl;
        std::streamsize size = file.gcount();
        if(size > 0)
        {
            ssize_t sen = send(link_sock, ass.data(), size, 0);
            // std::cout << sen << std::endl;
            if(sen < 0)
            {
                perror("File_Send Error");
                return;
            }
        }
        // std::cout << "sss" << std::endl;
    }
    file.close();
    close(link_sock);
            
}

void FTPClient::List()
{
    char lis[MAX_NUM];
    while(1)
    {
        ssize_t lis_l = recv(link_sock, lis, sizeof(lis) - 1, 0);
        if(lis_l < 0)
        {
            perror("LIST_Recive Error");
            return;
        }
        else if(lis_l == 0)
        break;
        lis[lis_l] = '\0';
        std::cout << lis << std::endl;
    }

}

void FTPClient::openPassiveMode(int a)
{
    p1 = 0;
    p2 = 0;
    memset(ass.data(), 0, sizeof(ass));
    // std::cout << a << std::endl;
    for(int i = 0; i < a; i++)
    {
        int n = 0;
        int d = 0;
        //std::cout << arr[i] << std::endl;
        if(arr[i++] == '(')
        {
            //std::cout << a << std::endl;
            while(i < a)
            {
                //std::cout << arr[i] << std::endl;
                if(arr[i] <= '9' && arr[i] >= '0')
                {
                    //std::cout << arr[i] << std::endl;
                    ass[n] = arr[i];
                    n++;
                    i++;
                }
                else if(arr[i] == ',')
                {
                    if(d == 3)
                    {
                        ass[n] = '\0';
                        i++;
                        //std::cout << ass << std::endl;
                        break;
                    }
                    ass[n] = '.';
                    n++;
                    i++;
                    d++;
                }
            }
            //std::cout << ass << std::endl;
            int t = 1;
            while(i < a)
            {
                //std::cout << arr[i] << std::endl;
                if(arr[i] <= '9' && arr[i] >= '0')
                {
                    if(t == 1)
                    {
                        //std::cout << arr[i] << std::endl;
                        p1 = p1 * 10 + (arr[i] - '0'); 
                        i++;
                    }
                    else if(t == 2)
                    {
                        p2 = p2 * 10 + (arr[i] - '0');
                        i++;
                    }
                }
                else if(arr[i] == ',')
                {
                    t = 2;
                    i++;
                }
                
                if(arr[i] == ')')
                break;
            }
        }
        //break;
        // std::cout << arr.data() << std::endl;
    }
    // std::cout << ass << std::endl;
    int p = p1 * 256 + p2;
    // std::cout << "p: " << p << std::endl;
    // std::cout << ass.data() << std::endl;
    link_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(link_sock < 0)
    {
        perror("Link_Socket Error");
        return;
    }
    sockaddr_in link;
    link.sin_port = htons(p);
    link.sin_family = AF_INET;
    inet_pton(AF_INET, ass.data(), &link.sin_addr.s_addr);
    Connect(link_sock, link);
    // std::cout << "success" << std::endl;
}

bool FTPClient::Recive()
{
    int a = recv(_sockfd, arr.data(), arr.size(), 0);
    if(a < 0)
    {
        return false;
    }
    else
    arr[a] = '\0';
    // std::cout << arr.data() << std::endl;
    return true;
}

void FTPClient::Connect(int sockfd, sockaddr_in cli)
{
    socklen_t l = sizeof(cli);
    if(connect(sockfd, (sockaddr*)&cli, l) < 0)
    {
        perror("Connect Error");
        close(sockfd);
        return;
    }
}

bool FTPClient::Send(int sockfd, const std::string& msg)
{
    // std::string send_buf = msg + "\r\n";

    if(send(sockfd, msg.data(), msg.size(), 0) < 0)
    {
        perror("Send_string Error");
        return false;
    }

    // send_buf.pop_back();
    // send_buf.pop_back();
    return true;
}

