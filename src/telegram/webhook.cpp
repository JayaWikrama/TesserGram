#include <stdexcept>
#include <iostream>
#include <cstring>
#include <thread>
#include "telegram.hpp"
#include "request.hpp"
#include "json-parser.hpp"
#include "debug.hpp"

extern "C" {
    #include "mongoose.h"
}

static Telegram *hook = nullptr;

static void webhookHandler(struct mg_connection *c, int ev, void *ev_data){
    if (ev != MG_EV_HTTP_MSG) return;

    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    std::string body(hm->body.buf, hm->body.len);
    bool isThreadRun = (hook->message.getMessage() != nullptr);
    hook->message.enqueue(body);

    mg_http_reply(c, 200,
        "Content-Type: application/json\r\n",
        "{%m:%m,%m:{%m:%m}}",
        MG_ESC("status"), MG_ESC("success"),
        MG_ESC("data"), MG_ESC("message"), MG_ESC("message received"));

    if (hook->webhookCallback && isThreadRun == false){
        void *(*callback)(Telegram &, void *) = (void *(*)(Telegram &, void *))hook->webhookCallback;
        std::thread([callback]() {
            callback(*hook, hook->webhookCallbackData);
        }).detach();
    }
}

bool Telegram::apiSetWebhook(const std::string& url, const std::string& secretToken, const std::string& allowedUpdates, unsigned short maxConnection){
    JsonBuilder json;
    json["url"] = url;
    if (secretToken.length()) json["secret_token"] = secretToken;
    if (allowedUpdates.length()) json["allowed_updates"] = allowedUpdates;
    if (maxConnection > 0) json["max_connections"] = maxConnection;
    Request req(TELEGRAM_BASE_URL, this->token, Request::SET_WEBHOOK, json.dump());
    if (req.isSuccess()){
        if (this->debug){
            Debug *dbg = (Debug *) this->debug;
            dbg->log(Debug::INFO, __PRETTY_FUNCTION__, "success\n");
        }
        return true;
    }
    return false;
}

bool Telegram::apiSetWebhook(const std::string& url, const std::string& secretToken, const std::string& allowedUpdates){
    return this->apiSetWebhook(url, secretToken, allowedUpdates, 40);
}

bool Telegram::apiSetWebhook(const std::string& url, unsigned short maxConnection){
    return this->apiSetWebhook(url, "", "", maxConnection);
}

bool Telegram::apiSetWebhook(const std::string& url){
    return this->apiSetWebhook(url, "", "", 40);
}

bool Telegram::apiUnsetWebhook(){
    Request req(TELEGRAM_BASE_URL, this->token, Request::UNSET_WEBHOOK, std::string("{\"drop_pending_updates\":true}"));
    if (req.isSuccess()){
        if (this->debug){
            Debug *dbg = (Debug *) this->debug;
            dbg->log(Debug::INFO, __PRETTY_FUNCTION__, "success\n");
        }
        return true;
    }
    return false;
}

void Telegram::setWebhookCallback(void (*__callback)(Telegram &, void *), void *data){
    this->webhookCallback = (void *) __callback;
    this->webhookCallbackData = data;
}

void Telegram::servWebhook(){
    hook = this;
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8443", webhookHandler, NULL);
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
}
