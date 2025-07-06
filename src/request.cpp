#include "fetch-api.hpp"
#include "request.hpp"

#define CONNECTION_TIMEOUT 10
#define ALL_TIMEOUT 15

static const char *reqStr[] = {
    "getMe",
    "getUpdates",
    "sendMessage",
    "editMessageText",
    "sendChatAction",
    "getFile",
    "",
    "",
    "sendPhoto",
    "sendAudio",
    "sendVideo",
    "sendAnimation",
    "sendVoice",
    "sendDocument",
    "setWebhook",
    "deleteWebhook"
};

Request::Request(const std::string &url, const std::string &token, Request::REQUEST_t req){
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[req];
    FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
    api.enableDebug(token);
    this->success = api.get();
    if (this->success){
        this->response = api.getPayload();
    }
}

Request::Request(const std::string &url, const std::string &token, Request::REQUEST_t req, const std::string &data){
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[req];
    FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
    api.enableDebug(token);
    api.insertHeader("Content-Type", "application/json");
    api.setBody(data);
    this->success = api.post();
    if (this->success){
        this->response = api.getPayload();
    }
}

Request::Request(const std::string &url, const std::string &token, Request::REQUEST_t req, const JsonBuilder &data){
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[req];
    FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
    api.enableDebug(token);
    api.setBody(data.dump());
    this->success = api.sendFile();
    if (this->success){
        this->response = api.getPayload();
    }
}

Request::Request(const std::string &url, const std::string &token, Request::REQUEST_t req, const std::string &ref, std::vector<unsigned char> &data){
    std::string mediaPath = ref;
    this->success = false;
    if (req == Request::DOWNLOAD_MEDIA_BY_FILE_ID){
        this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[Request::GET_MEDIA_PATH];
        FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
        api.enableDebug(token);
        JsonBuilder data;
        data["file_id"] = ref;
        api.setBody(data.dump());
        this->success = api.post();
        if (this->success){
            JsonObject json(api.getPayload());
            mediaPath = json["result->file_path"].getString();
            this->response = mediaPath;
            req = Request::DOWNLOAD_MEDIA_BY_PATH;
        }
        else {
            return;
        }
    }
    if (req == Request::DOWNLOAD_MEDIA_BY_PATH){
        this->url = url + (url.at(url.length() - 1) == '/' ? "file/bot" : "/file/bot") + token + "/" + mediaPath;
        FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
        api.enableDebug(token);
        this->success = api.download(data);
        if (this->success){
            this->response = mediaPath;
        }
    }
    return;
}

Request::~Request(){

}

bool Request::isSuccess(){
    return this->success;
}

const std::string& Request::getResponse() const {
    return this->response;
}