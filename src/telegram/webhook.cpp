#include <stdexcept>
#include <iostream>
#include <cstring>
#include <thread>
#include "telegram.hpp"
#include "request.hpp"
#include "utils/include/debug.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"

extern "C"
{
#include "mongoose.h"
}

static Telegram *hook = nullptr;

static void webhookHandler(struct mg_connection *c, int ev, void *ev_data)
{
    if (ev != MG_EV_HTTP_MSG)
        return;

    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    std::string body(hm->body.buf, hm->body.len);
    bool isThreadRun = (hook->messages.size() > 0);
    try
    {
        hook->messages.emplace_back();
        hook->messages.back().parse(body);
    }
    catch (const std::exception &e)
    {
        hook->messages.pop_back();
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "failed to parse message: %s!\n", e.what());
    }

    mg_http_reply(c, 200,
                  "Content-Type: application/json\r\n",
                  "{%m:%m,%m:{%m:%m}}",
                  MG_ESC("status"), MG_ESC("success"),
                  MG_ESC("data"), MG_ESC("message"), MG_ESC("message received"));

    if (hook->webhookCallback.ready() && isThreadRun == false)
    {
        std::thread([]()
                    { hook->webhookCallback.exec(*hook); })
            .detach();
    }
}

bool Telegram::apiSetWebhook(const std::string &url, const std::string &secretToken, const std::string &allowedUpdates, unsigned short maxConnection)
{
    nlohmann::json json;
    json["url"] = url;
    if (secretToken.length())
        json["secret_token"] = secretToken;
    if (allowedUpdates.length())
        json["allowed_updates"] = allowedUpdates;
    if (maxConnection > 0)
        json["max_connections"] = maxConnection;
    Request req(TELEGRAM_BASE_URL, this->token, Request::SET_WEBHOOK, json.dump());
    if (req.isSuccess())
    {

        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}

bool Telegram::apiSetWebhook(const std::string &url, const std::string &secretToken, const std::string &allowedUpdates)
{
    return this->apiSetWebhook(url, secretToken, allowedUpdates, 40);
}

bool Telegram::apiSetWebhook(const std::string &url, unsigned short maxConnection)
{
    return this->apiSetWebhook(url, "", "", maxConnection);
}

bool Telegram::apiSetWebhook(const std::string &url)
{
    return this->apiSetWebhook(url, "", "", 40);
}

bool Telegram::apiUnsetWebhook()
{
    Request req(TELEGRAM_BASE_URL, this->token, Request::UNSET_WEBHOOK, std::string("{\"drop_pending_updates\":true}"));
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}

void Telegram::setWebhookCallback(const Function &func)
{
    this->webhookCallback = func;
}

void Telegram::servWebhook()
{
    hook = this;
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8443", webhookHandler, NULL);
    for (;;)
    {
        mg_mgr_poll(&mgr, 1000);
    }
}
