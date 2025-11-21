#include "type.hpp"
#include "nlohmann/json.hpp"
#include "json-validator.hpp"
#include "utils/include/debug.hpp"

namespace
{
    static const std::array<std::string, 7> chatActionNames = {
        "typing",
        "upload_photo",
        "record_video",
        "upload_video",
        "record_voice",
        "upload_voice",
        "upload_document"};

    static const std::string unknownChatActionName = "unknown";
}

Chat::Chat()
{
    this->isForum = false;
    this->type = Chat::Type::PRIVATE;
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
        JSONValidator jval(__FILE__, __LINE__, __func__);

        jval.validate<std::string>(json, "type")
            .onValid(
                [this](const nlohmann::json &jsonType)
                {
                    std::string chatType = jsonType.get<std::string>();
                    if (chatType.compare("group") == 0)
                        this->type = Chat::Type::GROUP;
                    else if (chatType.compare("supergroup") == 0)
                        this->type == Chat::Type::SUPERGROUP;
                    else if (chatType.compare("channel") == 0)
                        this->type = Chat::Type::CHANNEL;
                    else
                        this->type = Chat::Type::PRIVATE;
                });

        this->id = jval.get<long long>(json, "id");
        this->firstName = jval.get<std::string>(json, "first_name");

        jval.validate<std::string>(json, "last_name")
            .onValid(
                [this](const nlohmann::json &jsonLastName)
                {
                    this->lastName = jsonLastName.get<std::string>();
                });

        this->username = jval.get<std::string>(json, "username");

        if (this->type != Chat::Type::PRIVATE)
            this->title = jval.get<std::string>(json, "title");
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
    this->type = Chat::Type::PRIVATE;
    this->id = 0;
    this->firstName.clear();
    this->lastName.clear();
    this->username.clear();
    this->title.clear();
}

const std::string &Chat::actionToString(const Chat::Action &action)
{
    std::size_t index = static_cast<std::size_t>(action);

    if (index < chatActionNames.size())
    {
        return chatActionNames[index];
    }
    return unknownChatActionName;
}