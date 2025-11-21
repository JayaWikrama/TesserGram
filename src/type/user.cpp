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
        JSONValidator jval(__FILE__, __LINE__, __func__);
        this->isBot = jval.get<bool>(json, "is_bot");
        this->id = jval.get<long long>(json, "id");
        this->firstName = jval.get<std::string>(json, "first_name");

        jval.validate<std::string>(json, "last_name")
            .onValid(
                [&](const nlohmann::json &jsonLastName)
                {
                    this->lastName = jsonLastName.get<std::string>();
                });

        this->username = jval.get<std::string>(json, "username");

        jval.validate<std::string>(json, "language_code")
            .onValid(
                [&](const nlohmann::json &jsonLangCode)
                {
                    this->languageCode = jsonLangCode.get<std::string>();
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

void User::reset()
{
    this->isBot = false;
    this->id = 0;
    this->firstName.clear();
    this->lastName.clear();
    this->username.clear();
    this->languageCode.clear();
}