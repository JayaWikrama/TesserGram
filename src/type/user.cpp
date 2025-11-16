#include "type.hpp"
#include "nlohmann/json.hpp"
#include "json-validator.hpp"
#include "utils/include/debug.hpp"

User::User()
{
    this->isBot = false;
    this->id = 0;
}

User::~User()
{
}

bool User::empty() const
{
    return (this->id == 0);
}

bool User::parse(const nlohmann::json &json)
{
    try
    {
        JSONValidator jvalidator(__FILE__, __LINE__, __func__);
        this->isBot = jvalidator.get<bool>(json, "is_bot");
        this->id = jvalidator.get<long long>(json, "id");
        this->firstName = jvalidator.get<std::string>(json, "first_name");
        if (json.contains("last_name"))
            this->lastName = jvalidator.get<std::string>(json, "last_name");
        this->username = jvalidator.get<std::string>(json, "username");
        if (json.contains("language_code"))
            this->languageCode = jvalidator.get<std::string>(json, "language_code");
        return true;
    }
    catch (const std::exception &e)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "parse failed: %s\n", e.what());
    }
    this->reset();
    return false;
}

void User::reset()
{
    this->isBot = false;
    this->id = 0;
    this->firstName.clear();
    this->lastName.clear();
    this->username.clear();
    this->languageCode.clear();
}