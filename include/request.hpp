#ifndef __REQUEST_API_HPP__
#define __REQUEST_API_HPP__

#include <string>
#include "json-parser.hpp"

class Request {
    private:
        bool success;
        std::string url;
        std::string response;

    public:
        typedef enum __REQUEST_t {
            CONFIG = 0,
            UPDATES,
            SEND_MESSAGE,
            EDIT_MESSAGE_TEXT,
            SEND_CHAT_ACTION,
            GET_MEDIA_PATH,
            SEND_PHOTO,
            SEND_AUDIO,
            SEND_VIDEO,
            SEND_ANIMATION,
            SEND_VOICE,
            SEND_DOCUMENT,
            SET_WEBHOOK,
            UNSET_WEBHOOK
        } REQUEST_t;
        Request(const std::string &url, const std::string &token, REQUEST_t req);
        Request(const std::string &url, const std::string &token, REQUEST_t req, const std::string &data);
        Request(const std::string &url, const std::string &token, REQUEST_t req, const JsonBuilder &data);
        ~Request();
        bool isSuccess();
        const std::string& getResponse() const;
};

#endif