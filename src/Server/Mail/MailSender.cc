#include "MailSender.h"

MailSender::MailSender(const std::string& smtp_url,
                       const std::string& username, 
                       const std::string& password)
    : smtp_url_(smtp_url), username_(username), password_(password) {}

struct upload_status{
    size_t bytes_read = 0;
    std::string payload;
};

static size_t payloadSource(void* ptr, size_t size, size_t nmemb, void* userp){
    upload_status* upload = static_cast<upload_status*>(userp);
    size_t buffer_size = size*nmemb;
    size_t bytes_left = upload->payload.size() - upload->bytes_read;

    if(bytes_left == 0)
    return 0;

    size_t bytes_copy = (bytes_left < buffer_size)?bytes_left: buffer_size;
    memcpy(ptr, upload->payload.c_str() + upload->bytes_read, bytes_copy);
    upload->bytes_read += bytes_copy;
    return bytes_copy;
} 

bool MailSender::sendMail(const std::string& from,
                          const std::vector<std::string>& to,
                          const std::string& subject, 
                          const std::string& content){
    CURL* curl = curl_easy_init();
    if(!curl){
        std::cerr << "Init curl failed." << std::endl;
        return false;
    }

    std::stringstream ss;
    ss << "To: ";
    for(ssize_t i = 0; i < to.size(); i++){
        ss << to[i];
        if(i != to.size() - 1) ss << ",";
    }
    std::string s = ss.str();
    ss << "\r\n";
    ss << "From: " << from << "\r\n";
    ss << "Subject: " << subject << "\r\n";
    ss << "\r\n";
    ss << content << "\r\n";

    upload_status upload_ctx;
    upload_ctx.payload = ss.str();

    struct curl_slist* recipients = nullptr;
    // recipients = curl_slist_append(recipients, from.c_str());
    for(const auto& addr: to){
        recipients = curl_slist_append(recipients, addr.c_str());
    }

    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  // 打开详细日志，看看底层交互细节


    curl_easy_setopt(curl, CURLOPT_URL, smtp_url_.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_USERNAME, username_.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password_.c_str());
    curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");
    std::string mail_from = "<" + from + ">";
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mail_from.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    if(res != CURLE_OK){
        std::cerr << "curl_easy_perform failed: " << curl_easy_strerror(res) << "\n";
        return false;
    }

    return true;
}




// senfile