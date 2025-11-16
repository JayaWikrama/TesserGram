#include <stdexcept>
#include <iostream>
#include <cstring>
#include "telegram.hpp"
#include "polling-controller.hpp"
#include "utils/include/debug.hpp"

Telegram::Telegram() : controller(3000, 10000), messages(), mutex()
{
    this->id = 0;
    this->lastUpdateId = 0;
    this->name = "";
    this->username = "";
    this->token = "";
    this->webhookCallback = nullptr;
}

Telegram::Telegram(const std::string &token) : controller(3000, 10000), messages(), mutex()
{
    this->id = 0;
    this->lastUpdateId = 0;
    this->name = "";
    this->username = "";
    this->token = token;
    this->webhookCallback = nullptr;
}

Telegram::~Telegram()
{
}

void Telegram::setToken(const std::string &token)
{
    this->token = token;
}

long long Telegram::getId() const
{
    return this->id;
}

const std::string &Telegram::getName() const
{
    return this->name;
}

const std::string &Telegram::getUsername() const
{
    return this->username;
}

void Telegram::info() const
{
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Bot Id: %lli\n", this->getId());
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Bot Name: %s\n", this->getName().c_str());
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Bot Username: %s\n", this->getUsername().c_str());
}

bool Telegram::getUpdates(std::function<void(Telegram &, const NodeMessage &)> handler)
{
    if (this->apiGetUpdates())
    {
        std::lock_guard<std::mutex> guard(this->mutex);
        for (const NodeMessage &message : this->messages)
        {
            handler(*this, message);
        }
        this->messages.clear();
        return true;
    }
    return false;
}

void Telegram::getUpdatesPoll(std::function<void(Telegram &, const NodeMessage &)> handler)
{
    controller.run(
        [&]()
        {
            return this->getUpdates(handler);
        });
}