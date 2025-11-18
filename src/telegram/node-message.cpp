#include "utils/include/debug.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"
#include "node-message.hpp"

NodeMessage::NodeMessage() : callbackQuery(), message(), updateId(0) {}

NodeMessage::NodeMessage(const nlohmann::json &message)
{
    this->parse(message);
}

NodeMessage::~NodeMessage()
{
}

void NodeMessage::parse(const nlohmann::json &message)
{
    JSONValidator jvalidator(__FILE__, __LINE__, __func__);

    this->updateId = jvalidator.get<long long>(message, "update_id");

    if (message.contains("callback_query"))
    {
        const nlohmann::json &jsonCallbackQuery = jvalidator.getObject(message, "callback_query");
        this->callbackQuery.parse(jsonCallbackQuery);
    }

    else if (message.contains("message"))
    {
        const nlohmann::json &jsonMessage = jvalidator.getObject(message, "message");
        this->message.parse(jsonMessage);
    }

    if (this->message.empty() && this->callbackQuery.empty())
    {
        throw std::runtime_error("Unknown type: " + message.dump() + "!");
    }
}

void NodeMessage::display() const
{
    struct tm dtime;
    char dtimestr[64];
    if (this->callbackQuery.empty() == false)
    {
        time_t ctime = time(nullptr);
        memcpy(&dtime, localtime(&ctime), sizeof(dtime));
    }
    else
    {
        memcpy(&dtime, localtime(&(this->message.dtime)), sizeof(dtime));
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
    debug.log(Debug::INFO, __func__, "Update ID   : %lli [%s]\n", this->updateId, dtimestr);
    debug.log(Debug::INFO, __func__, "  Type      : %s\n", (this->callbackQuery.empty() ? "message" : "callback_query"));
    debug.log(Debug::INFO, __func__, "  Id        : %lli\n", (this->message.empty() == false ? this->message.id : this->callbackQuery.id));
    debug.log(Debug::INFO, __func__, "  Sender    : %s\n", (this->message.empty() == false ? this->message.from.username.c_str() : this->callbackQuery.message->from.username.c_str()));
    debug.log(Debug::INFO, __func__, "  Room Id   : %lli\n", (this->message.empty() == false ? this->message.chat.id : this->callbackQuery.message->chat.id));
    debug.log(Debug::INFO, __func__, "  Room Name : %s\n", (this->message.empty() == false ? this->message.chat.title.c_str() : this->callbackQuery.message->chat.title.c_str()));
    if (this->callbackQuery.empty())
    {
        if (this->message.text.length())
        {
            debug.log(Debug::INFO, __func__, "  Message   : %s\n", this->message.text.c_str());
        }
        if (this->message.media.size() == 1)
        {
            debug.log(Debug::INFO, __func__, "  Media T   : %s\n", this->message.media.at(0).getType().c_str());
            debug.log(Debug::INFO, __func__, "  Media Id  : %s\n", this->message.media.at(0).fileId.c_str());
        }
        else if (this->message.media.size() > 1)
        {
            debug.log(Debug::INFO, __func__, "  Media T   : %s\n", this->message.media.at(0).getType().c_str());
            for (int i = 0; i < this->message.media.size(); i++)
            {
                debug.log(Debug::INFO, __func__, "    M.id[%d] : %s\n", i, this->message.media.at(i).fileId.c_str());
            }
        }
    }
    else
    {
        debug.log(Debug::INFO, __func__, "  Data      : %s\n", this->callbackQuery.data.c_str());
    }
}

long long NodeMessage::getId() const
{
    return this->updateId;
}

const NodeMessage &NodeMessage::processMessage(std::function<void(const Message &)> handler) const
{
    if (!(this->message.empty()))
    {
        handler(this->message);
    }
    return *this;
}

const NodeMessage &NodeMessage::processCallbackQuery(std::function<void(const CallbackQuery &)> handler) const
{
    if (!(this->callbackQuery.empty()))
    {
        handler(this->callbackQuery);
    }
    return *this;
}