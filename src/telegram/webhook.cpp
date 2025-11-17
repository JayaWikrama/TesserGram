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

static void webhookHandler(struct mg_connection *c, int ev, void *ev_data)
{
    if (ev != MG_EV_HTTP_MSG)
        return;

    Telegram *hook = (Telegram *)c->fn_data;

    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    std::string body(hm->body.buf, hm->body.len);

    hook->parseGetUpdatesResponse(body);

    mg_http_reply(c, 200,
                  "Content-Type: application/json\r\n",
                  "{%m:%m,%m:{%m:%m}}",
                  MG_ESC("status"), MG_ESC("success"),
                  MG_ESC("data"), MG_ESC("message"), MG_ESC("message received"));

    hook->execWebhookCallback();
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
    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::SET_WEBHOOK, json.dump());
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
    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::UNSET_WEBHOOK, std::string("{\"drop_pending_updates\":true}"));
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}

void Telegram::setWebhookCallback(std::function<void(Telegram &, const NodeMessage &)> handler)
{
    this->webhookCallback = handler;
}

void Telegram::servWebhook()
{
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8443", webhookHandler, this);
    for (;;)
    {
        mg_mgr_poll(&mgr, 1000);
    }
}

void Telegram::execWebhookCallback()
{
    if (this->webhookCallback && !(this->messages.empty()))
    {
        for (const NodeMessage &message : this->messages)
        {
            this->webhookCallback(*this, message);
        }
        this->messages.clear();
    }
}