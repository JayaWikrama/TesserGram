#include "type.hpp"
#include "nlohmann/json.hpp"
#include "json-validator.hpp"
#include "utils/include/debug.hpp"

Message::Message()
{
    this->dtime = 0;
    this->id = 0;
    this->threadId = 0;
    this->replyToMessage = nullptr;
}

Message::~Message()
{
}

bool Message::empty() const
{
    return (this->id == 0);
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
                const nlohmann::json &jsonReplyToMessage = jvalidator.getObject(json, "reply_to_message");
                this->replyToMessage.reset(new Message());
                if (this->replyToMessage.get() != nullptr)
                {
                    if (this->replyToMessage->parse(jsonReplyToMessage) == false)
                    {
                        this->replyToMessage.reset();
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
    if (this->replyToMessage.get() != nullptr)
        this->replyToMessage.reset();
}
