#include <stdexcept>
#include <iostream>
#include <cstring>
#include "telegram.hpp"

Telegram::Telegram(const std::string &token) : messages(), webhookCallback()
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

long long Telegram::getId()
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