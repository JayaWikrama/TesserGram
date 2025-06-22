#include "fetch-api.hpp"
#include "request.hpp"

#define CONNECTION_TIMEOUT 3
#define ALL_TIMEOUT 10

static const char *reqStr[] = {
    "getMe",
    "getUpdates",
    "sendMessage"
};

Request::Request(const std::string &url, std::string &token, Request::REQUEST_t req){
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[req];
    FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
    api.enableDebug();
    this->success = api.get();
    if (this->success){
        this->response = api.getPayload();
    }
}

Request::Request(const std::string &url, std::string &token, Request::REQUEST_t req, std::string &data){
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[req];
    FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
    api.enableDebug();
    api.insertHeader("Content-Type", "application/json");
    api.setBody(data);
    this->success = api.post();
    if (this->success){
        this->response = api.getPayload();
    }
}

Request::~Request(){

}

bool Request::isSuccess(){
    return this->success;
}

const std::string& Request::getResponse() const {
    return this->response;
}