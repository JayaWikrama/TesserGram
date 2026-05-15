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
    JSONValidator jval(__FILE__, __LINE__, __func__);

    this->updateId = jval.get<long long>(message, "update_id");

    jval.object(message, "callback_query")
        .onValid(
            [this](const nlohmann::json &jsonCallbackQuery)
            {
                this->callbackQuery.parse(jsonCallbackQuery);
            })
        .onNotFound(
            [this](const nlohmann::json &jalt, const std::string &err)
            {
                JSONValidator mval(__FILE__, __LINE__, __func__);
                mval.object(jalt, "message")
                    .onValid(
                        [this](const nlohmann::json &jsonMessage)
                        {
                            this->message.parse(jsonMessage);
                        });
            });

    if (this->message.empty() && this->callbackQuery.empty())
    {
        throw std::runtime_error("Unknown type: " + message.dump() + "!");
    }
}

static void displayCallbackQuery(const CallbackQuery &cq, long long updateId, const char *dtimestr)
{
    long long roomId = 0;
    const char *roomName = "<n/a>";
    if (cq.message != nullptr)
    {
        roomId = cq.message->chat.id;
        roomName = cq.message->chat.title.c_str();
    }
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Update ID   : %lli [%s]\n", updateId, dtimestr);
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Type      : callback_query\n");
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Id        : %lli\n", cq.id);
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Sender    : %s\n", cq.from.username.c_str());
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Room Id   : %lli\n", roomId);
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Room Name : %s\n", roomName);
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Data      : %s\n", cq.data.c_str());
}

void NodeMessage::display() const
{
    struct tm dtime;
    char dtimestr[64];

    if (!this->callbackQuery.empty())
    {
        // callback_query carries no server timestamp; substitute local wall-clock time for display
        time_t ctime = time(nullptr);
        localtime_r(&ctime, &dtime);
        strftime(dtimestr, sizeof(dtimestr), "%Y-%m-%d %H:%M:%S", &dtime);
        displayCallbackQuery(this->callbackQuery, this->updateId, dtimestr);
        return;
    }

    localtime_r(&(this->message.dtime), &dtime);
    strftime(dtimestr, sizeof(dtimestr), "%Y-%m-%d %H:%M:%S", &dtime);

    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Update ID   : %lli [%s]\n", this->updateId, dtimestr);
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Type      : message\n");
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Id        : %lli\n", this->message.id);
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Sender    : %s\n", this->message.from.username.c_str());
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Room Id   : %lli\n", this->message.chat.id);
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Room Name : %s\n", this->message.chat.title.c_str());
    if (this->message.text.length())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Message   : %s\n", this->message.text.c_str());
    }
    if (this->message.media.size() == 1)
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Media T   : %s\n", this->message.media.at(0).getType().c_str());
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Media Id  : %s\n", this->message.media.at(0).fileId.c_str());
    }
    else if (this->message.media.size() > 1)
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "  Media T   : %s\n", this->message.media.at(0).getType().c_str());
        for (std::size_t i = 0; i < this->message.media.size(); i++)
        {
            Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "    M.id[%zu] : %s\n", i, this->message.media.at(i).fileId.c_str());
        }
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