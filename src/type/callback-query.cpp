#include "type.hpp"
#include "nlohmann/json.hpp"
#include "json-validator.hpp"
#include "utils/include/debug.hpp"

CallbackQuery::CallbackQuery()
{
    this->id = 0;
    this->message = nullptr;
}

CallbackQuery::~CallbackQuery()
{
}

bool CallbackQuery::empty() const
{
    return (this->id == 0);
}

bool CallbackQuery::parse(const nlohmann::json &json)
{
    try
    {
        JSONValidator jval(__FILE__, __LINE__, __func__);

        jval.validate<long long>(json, "id")
            .onValid(
                [&](const nlohmann::json &jid)
                {
                    this->id = jid.get<long long>();
                })
            .onInvalid(
                [&](const nlohmann::json &jid, const std::string &err)
                {
                    this->id = std::stoll(jid.get<std::string>());
                });

        jval.validate<long long>(json, "chat_instance")
            .onValid(
                [&](const nlohmann::json &jci)
                {
                    this->chatInstance = jci.get<long long>();
                })
            .onInvalid(
                [&](const nlohmann::json &jci, const std::string &err)
                {
                    this->chatInstance = std::stoll(jci.get<std::string>());
                });

        this->data = jval.get<std::string>(json, "data");

        const nlohmann::json &jsonFrom = jval.getObject(json, "from");

        this->from.parse(jsonFrom);

        jval.object(json, "message")
            .onValid(
                [this](const nlohmann::json &jmsg)
                {
                    this->message.reset(new Message);
                    if (this->message.get() != nullptr)
                    {
                        if (this->message->parse(jmsg) == false)
                        {
                            this->message.reset();
                        }
                    }
                });

        return true;
    }
    catch (const std::exception &e)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "parse failed: %s\n", e.what());
    }
    this->reset();
    return false;
}

void CallbackQuery::reset()
{
    this->id = 0;
    this->chatInstance.clear();
    this->data.clear();
    this->from.reset();
    if (this->message.get() != nullptr)
        this->message.reset();
}