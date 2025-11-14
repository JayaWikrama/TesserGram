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
    if (this->message)
        delete this->message;
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
    if (this->message)
        delete this->message;
    this->message = nullptr;
}