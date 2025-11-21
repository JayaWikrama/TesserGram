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
        JSONValidator jval(__FILE__, __LINE__, __func__);

        jval.validate<std::string>(json, "date")
            .onValid(
                [this](const nlohmann::json &jsonDate)
                {
                    this->dtime = jsonDate.get<long>();
                })
            .onInvalid(
                [this]()
                {
                    this->dtime = 0;
                });

        this->id = jval.get<long long>(json, "message_id");

        jval.validate<std::string>(json, "message_thread_id")
            .onValid(
                [this](const nlohmann::json &jsonMTId)
                {
                    this->threadId = jsonMTId.get<long long>();
                })
            .onInvalid(
                [this]()
                {
                    this->threadId = 0;
                });

        jval.validate<std::string>(json, "text")
            .onValid(
                [this](const nlohmann::json &jsonText)
                {
                    this->text = jsonText.get<std::string>();
                })
            .onInvalid(
                [this]()
                {
                    this->text.clear();
                });

        jval.validate<std::string>(json, "caption")
            .onValid(
                [this](const nlohmann::json &jsonCaption)
                {
                    this->caption = jsonCaption.get<std::string>();
                })
            .onInvalid(
                [this]()
                {
                    this->caption.clear();
                });
        const nlohmann::json &jsonFrom = jval.getObject(json, "from");
        const nlohmann::json &jsonChat = jval.getObject(json, "chat");
        this->from.parse(jsonFrom);
        this->chat.parse(jsonChat);

        jval.validate<std::string>(json, "reply_to_message")
            .onValid(
                [this](const nlohmann::json &jsonReplyToMessage)
                {
                    this->replyToMessage.reset(new Message());
                    if (this->replyToMessage.get() != nullptr)
                    {
                        if (this->replyToMessage->parse(jsonReplyToMessage) == false)
                        {
                            this->replyToMessage.reset();
                        }
                    }
                })
            .onInvalid(
                [this]()
                {
                    this->replyToMessage.reset();
                });

        Media::typeIteration(
            [&](const Media::Type &type, const std::string &name)
            {
                if (json.contains(name))
                {
                    const nlohmann::json j = json[name];
                    if (j.is_array())
                    {
                        for (const nlohmann::json &el : j)
                        {
                            Media md;
                            if (md.parse(type, el))
                            {
                                this->media.push_back(md);
                            }
                        }
                    }
                    else if (j.is_object())
                    {
                        Media md;
                        if (md.parse(type, j))
                        {
                            this->media.push_back(md);
                        }
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
