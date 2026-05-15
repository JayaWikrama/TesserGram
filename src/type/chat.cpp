#include <unordered_map>
#include "type.hpp"
#include "nlohmann/json.hpp"
#include "json-validator.hpp"
#include "utils/include/debug.hpp"

namespace
{
    // Canonical mapping: Chat::Action → wire string. Chat::actionToString() is
    // the sole public accessor — no other translation unit should duplicate this.
    constexpr std::size_t chatActionCount =
        static_cast<std::size_t>(Chat::Action::UPLOAD_DOCUMENT) + 1;

    static_assert(chatActionCount == 7,
        "chatActionNames out of sync with Chat::Action enum — update both together");

    static const std::array<std::string, chatActionCount> chatActionNames = {
        "typing",
        "upload_photo",
        "record_video",
        "upload_video",
        "record_voice",
        "upload_voice",
        "upload_document"};

    static const std::string unknownChatActionName = "unknown";

    static const std::unordered_map<std::string, Chat::Type> chatTypeMap = {
        {"private",    Chat::Type::PRIVATE},
        {"group",      Chat::Type::GROUP},
        {"supergroup", Chat::Type::SUPERGROUP},
        {"channel",    Chat::Type::CHANNEL}};
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

bool Chat::parsePrivateFields(const nlohmann::json &json)
{
    JSONValidator jval(__FILE__, __LINE__, __func__);
    this->firstName = jval.get<std::string>(json, "first_name");
    jval.validate<std::string>(json, "last_name")
        .onValid([this](const nlohmann::json &jln) { this->lastName = jln.get<std::string>(); });
    jval.validate<std::string>(json, "username")
        .onValid([this](const nlohmann::json &jun) { this->username = jun.get<std::string>(); });
    this->title = this->firstName;
    if (!this->lastName.empty())
        this->title += " " + this->lastName;
    return true;
}

bool Chat::parseGroupFields(const nlohmann::json &json)
{
    JSONValidator jval(__FILE__, __LINE__, __func__);
    this->title = jval.get<std::string>(json, "title");
    jval.validate<std::string>(json, "username")
        .onValid([this](const nlohmann::json &jun) { this->username = jun.get<std::string>(); });
    return true;
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
                    auto it = chatTypeMap.find(chatType);
                    this->type = (it != chatTypeMap.end()) ? it->second : Chat::Type::PRIVATE;
                });

        this->id = jval.get<long long>(json, "id");

        if (this->type == Chat::Type::PRIVATE)
            return this->parsePrivateFields(json);
        else
            return this->parseGroupFields(json);
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