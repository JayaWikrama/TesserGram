#include <string>
#include "webhook-server.hpp"
#include "telegram.hpp"

extern "C"
{
#include "mongoose.h"
}

static void webhookHandler(struct mg_connection *c, int ev, void *ev_data)
{
    if (ev != MG_EV_HTTP_MSG)
        return;

    Telegram *hook = static_cast<Telegram *>(c->fn_data);
    struct mg_http_message *hm = static_cast<struct mg_http_message *>(ev_data);
    std::string body(hm->body.buf, hm->body.len);

    hook->parseGetUpdatesResponse(body);

    mg_http_reply(c, 200,
                  "Content-Type: application/json\r\n",
                  "{%m:%m,%m:{%m:%m}}",
                  MG_ESC("status"), MG_ESC("success"),
                  MG_ESC("data"), MG_ESC("message"), MG_ESC("message received"));

    hook->execWebhookCallback();
}

WebhookServer::WebhookServer() : running(false) {}

WebhookServer::~WebhookServer()
{
    stop();
}

void WebhookServer::run(const std::string &listenAddr, Telegram *tg)
{
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, listenAddr.c_str(), webhookHandler, tg);
    running = true;
    while (running)
    {
        mg_mgr_poll(&mgr, 100);
    }
    mg_mgr_free(&mgr);
}

void WebhookServer::stop()
{
    running = false;
}
