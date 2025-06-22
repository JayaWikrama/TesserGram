#ifndef __REQUEST_API_HPP__
#define __REQUEST_API_HPP__

#include <string>

class Request {
    private:
        bool success;
        std::string url;
        std::string response;

    public:
        typedef enum __REQUEST_t {
            CONFIG = 0,
            UPDATES,
            SEND_MESSAGE
        } REQUEST_t;
        Request(const std::string &url, std::string &token, REQUEST_t req);
        Request(const std::string &url, std::string &token, REQUEST_t req, std::string &data);
        ~Request();
        bool isSuccess();
        const std::string& getResponse() const;
};

#endif