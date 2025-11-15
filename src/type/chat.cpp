#include "type.hpp"
#include "nlohmann/json.hpp"
#include "json-validator.hpp"
#include "utils/include/debug.hpp"

Chat::Chat()
{
    this->isForum = false;
    this->type = Chat::PRIVATE;
    this->id = 0;
}

Chat::~Chat()
{
}

bool Chat::empty() const
{
    return (this->id == 0);
}

bool Chat::parse(const nlohmann::json &json)
{
    try
    {
        JSONValidator jvalidator(__FILE__, __LINE__, __func__);

        if (json.contains("type"))
        {
            std::string chatType = jvalidator.get<std::string>(json, "type");
            if (chatType.compare("group") == 0)
                this->type = Chat::GROUP;
            else if (chatType.compare("supergroup") == 0)
                this->type == Chat::SUPERGROUP;
            else if (chatType.compare("channel") == 0)
                this->type = Chat::CHANNEL;
        }

        this->id = jvalidator.get<long long>(json, "id");
        this->firstName = jvalidator.get<std::string>(json, "first_name");
        if (json.contains("last_name"))
            this->lastName = jvalidator.get<std::string>(json, "last_name");
        this->username = jvalidator.get<std::string>(json, "username");

        if (this->type != Chat::PRIVATE)
            this->title = jvalidator.get<std::string>(json, "title");
        else
        {
            this->title = this->firstName;
            if (this->lastName.length() > 0)
            {
                this->title += " " + this->lastName;
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

void Chat::reset()
{
    this->isForum = false;
    this->type = Chat::PRIVATE;
    this->id = 0;
    this->firstName.clear();
    this->lastName.clear();
    this->username.clear();
    this->title.clear();
}
