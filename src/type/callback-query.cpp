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
        JSONValidator jvalidator(__FILE__, __LINE__, __func__);

        this->id = jvalidator.get<long long>(json, "id");
        this->chatInstance = jvalidator.get<std::string>(json, "chat_instance");
        this->data = jvalidator.get<std::string>(json, "data");

        const nlohmann::json &jsonFrom = jvalidator.getObject(json, "from");

        this->from.parse(jsonFrom);

        if (json.contains("message"))
        {
            const nlohmann::json &jsonMessage = jvalidator.getObject(json, "message");
            this->message.reset(new Message());
            if (this->message.get() != nullptr)
            {
                if (this->message->parse(jsonMessage) == false)
                {
                    this->message.reset();
                }
            }
        }

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