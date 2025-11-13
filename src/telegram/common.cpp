#include <stdexcept>
#include <iostream>
#include <cstring>
#include "utils/include/debug.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"
#include "telegram.hpp"

NodeMessage::NodeMessage(const std::string &message)
{
    nlohmann::json json = nlohmann::json::parse(message);
    JSONValidator jvalidator(__FILE__, __LINE__, __func__);

    this->updateId = jvalidator.get<long long>(json, "update_id");

    if (json.contains("callback_query"))
    {
        const nlohmann::json &jsonCallbackQuery = jvalidator.getObject(json, "callback_query");
        this->callbackQuery = new CallbackQuery();
        if (this->callbackQuery)
        {
            if (this->callbackQuery->parse(jsonCallbackQuery) == false)
            {
                delete this->callbackQuery;
                this->callbackQuery = nullptr;
            }
        }
    }

    else if (json.contains("message"))
    {
        const nlohmann::json &jsonMessage = jvalidator.getObject(json, "message");
        this->message = new Message();
        if (this->message)
        {
            if (this->message->parse(jsonMessage) == false)
            {
                delete this->message;
                this->message = nullptr;
            }
        }
    }

    if (this->message == nullptr && this->callbackQuery == nullptr)
    {
        throw std::runtime_error("Unknown type: " + message + "!");
    }
    this->next = nullptr;
}

void NodeMessage::display() const
{
    struct tm dtime;
    char dtimestr[64];
    if (this->callbackQuery)
    {
        time_t ctime = time(nullptr);
        memcpy(&dtime, localtime(&ctime), sizeof(dtime));
    }
    else
    {
        memcpy(&dtime, localtime(&(this->message->dtime)), sizeof(dtime));
    }
    sprintf(dtimestr,
            "%04d-%02d-%02d %02d:%02d:%02d",
            dtime.tm_year + 1900,
            dtime.tm_mon + 1,
            dtime.tm_mday,
            dtime.tm_hour,
            dtime.tm_min,
            dtime.tm_sec);
    dtimestr[19] = 0x00;
    Debug debug(0);
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "Update ID   : %lli [%s]\n", this->updateId, dtimestr);
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Type      : %s\n", (this->callbackQuery == nullptr ? "message" : "callback_query"));
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Id        : %lli\n", (this->message ? this->message->id : this->callbackQuery->id));
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Sender    : %s\n", (this->message ? this->message->from.username.c_str() : this->callbackQuery->message->from.username.c_str()));
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Room Id   : %lli\n", (this->message ? this->message->chat.id : this->callbackQuery->message->chat.id));
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Room Name : %s\n", (this->message ? this->message->chat.title.c_str() : this->callbackQuery->message->chat.title.c_str()));
    if (this->message)
    {
        if (this->message->text.length())
        {
            debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Message   : %s\n", this->message->text.c_str());
        }
        if (this->message->media.size() == 1)
        {
            debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Media T   : %s\n", this->message->media.at(0).getType().c_str());
            debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Media Id  : %s\n", this->message->media.at(0).fileId.c_str());
        }
        else if (this->message->media.size() > 1)
        {
            debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Media T   : %s\n", this->message->media.at(0).getType().c_str());
            for (int i = 0; i < this->message->media.size(); i++)
            {
                debug.log(Debug::INFO, __PRETTY_FUNCTION__, "    M.id[%d] : %s\n", i, this->message->media.at(i).fileId.c_str());
            }
        }
    }
    else
    {
        debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Data      : %s\n", this->callbackQuery->data.c_str());
    }
}

void Messages::enqueue(const std::string &message)
{
    NodeMessage *msg = nullptr;
    try
    {
        msg = new NodeMessage(message);
        if (this->first == nullptr)
        {
            this->first = msg;
            this->n++;
        }
        else
        {
            this->n++;
            this->end->next = msg;
        }
        this->end = msg;
    }
    catch (const std::runtime_error &e)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "Caught runtime_error: %s\n", e.what());
    }
}

void Messages::dequeue()
{
    if (this->first == nullptr)
        return;
    NodeMessage *tmp = this->first->next;
    delete this->first;
    this->first = tmp;
    this->n--;
    if (this->first == nullptr)
        this->end = nullptr;
}

Messages::Messages()
{
    this->n = 0;
    this->first = nullptr;
    this->end = nullptr;
}

Messages::~Messages()
{
    while (this->first)
        this->dequeue();
}

void Messages::clear()
{
    while (this->first)
        this->dequeue();
}

const NodeMessage *Messages::getMessage() const
{
    return this->first;
}

Telegram::Telegram(const std::string &token)
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