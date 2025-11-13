#include "type.hpp"
#include "nlohmann/json.hpp"
#include "json-validator.hpp"
#include "utils/include/debug.hpp"

struct MediaTypeEntry
{
    Media::TYPE_t type;
    const char *key;
};

static const MediaTypeEntry mediaTypes[] = {
    {Media::DOCUMENT, "document"},
    {Media::PHOTO, "photo"},
    {Media::ANIMATION, "animation"},
    {Media::STICKER, "sticker"},
    {Media::STORY, "story"},
    {Media::VIDEO, "video"},
    {Media::VIDEO_NOTE, "video_note"},
    {Media::VOICE, "voice"},
    {Media::AUDIO, "audio"},
    {Media::CONTACT, "contact"}};

User::User()
{
    this->isBot = false;
    this->id = 0;
}

User::~User()
{
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

Chat::Chat()
{
    this->isForum = false;
    this->type = Chat::PRIVATE;
    this->id = 0;
}

Chat::~Chat()
{
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

Media::Media()
{
    this->type = Media::DOCUMENT;
    this->fileSize = 0;
}

Media::~Media()
{
}

bool Media::parse(TYPE_t type, const nlohmann::json &json)
{
    try
    {
        JSONValidator jvalidator(__FILE__, __LINE__, __func__);

        this->type = type;

        if (json.contains("file_size"))
            this->fileSize = jvalidator.get<long long>(json, "file_size");
        else
            this->fileSize = 0;

        this->fileId = jvalidator.get<std::string>(json, "file_id");
        this->fileUniqueId = jvalidator.get<std::string>(json, "file_unique_id");

        if (json.contains("file_name"))
            this->fileName = jvalidator.get<std::string>(json, "file_name");

        return true;
    }
    catch (const std::exception &e)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "parse failed: %s\n", e.what());
    }
    this->reset();
    return false;
}

void Media::reset()
{
    this->type = Media::DOCUMENT;
    this->fileSize = 0;
    this->fileId.clear();
    this->fileUniqueId.clear();
    this->fileName.clear();
}

const std::string Media::getType()
{
    for (const auto &entry : mediaTypes)
    {
        if (entry.type == this->type)
            return std::string(entry.key);
    }
    return "unknown";
}

Message::Message()
{
    this->dtime = 0;
    this->id = 0;
    this->threadId = 0;
    this->replyToMessage = nullptr;
}

Message::~Message()
{
    if (this->replyToMessage)
        delete this->replyToMessage;
}

bool Message::parse(const nlohmann::json &json)
{
    try
    {
        JSONValidator jvalidator(__FILE__, __LINE__, __func__);

        if (json.contains("date"))
            this->dtime = jvalidator.get<long>(json, "date");
        else
            this->dtime = 0;

        this->id = jvalidator.get<long long>(json, "message_id");

        if (json.contains("message_thread_id"))
            this->threadId = jvalidator.get<long long>(json, "message_thread_id");
        else
            this->threadId = 0;

        if (json.contains("text"))
            this->text = jvalidator.get<std::string>(json, "text");
        if (json.contains("caption"))
            this->caption = jvalidator.get<std::string>(json, "caption");

        const nlohmann::json &jsonFrom = jvalidator.getObject(json, "from");
        const nlohmann::json &jsonChat = jvalidator.getObject(json, "chat");
        this->from.parse(jsonFrom);
        this->chat.parse(jsonChat);

        if (json.contains("reply_to_message"))
        {
            try
            {
                this->replyToMessage = new Message();
                const nlohmann::json &jsonReplyToMessage = jvalidator.getObject(json, "reply_to_message");
                if (this->replyToMessage)
                {
                    if (this->replyToMessage->parse(jsonReplyToMessage) == false)
                    {
                        delete this->replyToMessage;
                        this->replyToMessage = nullptr;
                    }
                }
            }
            catch (const std::exception &e)
            {
                Debug::log(Debug::WARNING, __FILE__, __LINE__, __func__, "parse \"reply to message\" failed: %s!\n", e.what());
            }
        }

        for (const struct MediaTypeEntry &entry : mediaTypes)
        {
            if (json.contains(entry.key) == false)
                continue;

            const nlohmann::json j = json[entry.key];
            if (j.is_array())
            {
                for (const nlohmann::json &el : j)
                {
                    Media md;
                    if (md.parse(entry.type, el))
                    {
                        this->media.push_back(md);
                    }
                }
            }
            else if (j.is_object())
            {
                Media md;
                if (md.parse(entry.type, j))
                {
                    this->media.push_back(md);
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

void Message::reset()
{
    this->dtime = 0;
    this->id = 0;
    this->threadId = 0;
    this->text.clear();
    this->caption.clear();
    this->from.reset();
    this->chat.reset();
    this->media.clear();
    if (this->replyToMessage)
        delete this->replyToMessage;
    this->replyToMessage = nullptr;
}

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