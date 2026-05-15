#ifndef __WEBHOOK_SERVER_HPP__
#define __WEBHOOK_SERVER_HPP__

#include <atomic>
#include <string>

class Telegram;

class WebhookServer
{
public:
    WebhookServer();
    ~WebhookServer();

    void run(const std::string &listenAddr, Telegram *tg);
    void stop();

private:
    std::atomic<bool> running;
};

#endif
