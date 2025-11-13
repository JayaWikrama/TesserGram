#include <stdexcept>
#include <iostream>
#include <cstring>
#include "telegram.hpp"
#include "request.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"
#include "utils/include/debug.hpp"

static const std::string ChatActionStr[] = {
    "typing",
    "upload_photo",
    "record_video",
    "upload_video",
    "record_voice",
    "upload_voice",
    "upload_document"};

bool Telegram::__parseGetUpdatesResponse(const std::string &buffer)
{
    try
    {
        nlohmann::json json = nlohmann::json::parse(buffer);
        JSONValidator jvalidator(__FILE__, __LINE__, __func__);

        nlohmann::json jsonResult = jvalidator.getArray(json, "result");

        long long updateId = 0;
        for (const nlohmann::json &el : jsonResult)
        {
            try
            {
                updateId = jvalidator.get<long long>(el, "update_id");
                this->message.enqueue(el.dump());
                if (this->lastUpdateId < updateId)
                    this->lastUpdateId = updateId;
            }
            catch (const std::exception &e)
            {
                Debug::log(Debug::WARNING, __FILE__, __LINE__, __func__, "skip: %s!\n", e.what());
            }
        }
        return true;
    }
    catch (const std::exception &e)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "parse failed: %s!\n", e.what());
    }
    return false;
}

bool Telegram::apiGetMe()
{
    Request req(TELEGRAM_BASE_URL, this->token, Request::CONFIG);
    if (req.isSuccess())
    {
        try
        {
            nlohmann::json json = nlohmann::json::parse(req.getResponse());
            JSONValidator jvalidator(__FILE__, __LINE__, __func__);

            nlohmann::json jsonResult = jvalidator.getObject(json, "result");

            this->id = jvalidator.get<long long>(jsonResult, "id", "result");
            this->name = jvalidator.get<std::string>(jsonResult, "first_name", "result");
            this->username = jvalidator.get<std::string>(jsonResult, "username", "result");
            return true;
        }
        catch (const std::exception &e)
        {
            Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "parse failed: %s!\n", e.what());
        }
    }
    return false;
}

bool Telegram::apiGetUpdates()
{
    if (this->lastUpdateId > 0)
    {
        std::string data = "{\"offset\":" + std::to_string(this->lastUpdateId + 1) + "}";
        Request req(TELEGRAM_BASE_URL, this->token, Request::UPDATES, data);
        if (req.isSuccess())
        {
            return this->__parseGetUpdatesResponse(req.getResponse());
        }
    }
    else
    {
        Request req(TELEGRAM_BASE_URL, this->token, Request::UPDATES);
        if (req.isSuccess())
        {
            return this->__parseGetUpdatesResponse(req.getResponse());
        }
    }
    return false;
}

bool Telegram::apiSendMessage(long long targetId, const std::string &message)
{
    nlohmann::json json;
    json["chat_id"] = targetId;
    json["text"] = message;
    Request req(TELEGRAM_BASE_URL, this->token, Request::SEND_MESSAGE, json.dump());
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}

bool Telegram::apiEditMessageText(long long targetId, long long messageId, const std::string &message)
{
    nlohmann::json json;
    json["chat_id"] = targetId;
    json["message_id"] = messageId;
    json["text"] = message;
    Request req(TELEGRAM_BASE_URL, this->token, Request::EDIT_MESSAGE_TEXT, json.dump());
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}

bool Telegram::apiSendChatAction(long long targetId, Telegram::ChatAction_t action)
{
    std::string data = "{\"chat_id\":" + std::to_string(targetId) + ",\"action\":\"" + ChatActionStr[action] + "\"}";
    Request req(TELEGRAM_BASE_URL, this->token, Request::SEND_CHAT_ACTION, data);
    if (req.isSuccess())
    {

        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}