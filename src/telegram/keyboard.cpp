#include <stdexcept>
#include <iostream>
#include <cstring>
#include "telegram.hpp"
#include "request.hpp"
#include "utils/include/debug.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"

TKeyboard::TKeyboard(TKeyboard::TKeyType_t type, const std::string &caption)
{
    this->type = type;
    this->caption = caption;
}

TKeyboard::~TKeyboard()
{
    // do nothing
}

bool TKeyboard::addButton(const std::string &button)
{
    if (this->type != TKeyboard::KEYBOARD)
        return false;
    try
    {
        nlohmann::json json = nlohmann::json::parse(button);
        if (json.is_array() == false)
            throw std::runtime_error(Error::fieldTypeInvalid(__FILE__, __LINE__, __func__, "array", "json"));
        this->buttons.push_back(button);
        return true;
    }
    catch (const std::exception &e)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "parse failed: %s!\n", e.what());
    }
    return false;
}

bool TKeyboard::addButton(TKeyboard::TValueType_t type, const std::string &text, const std::string &value)
{
    if (this->type != TKeyboard::INLINE_KEYBOARD)
        return false;
    if (type == TKeyboard::URL)
    {
        nlohmann::json json;
        json["text"] = text;
        json["url"] = value;
        this->buttons.push_back("[" + json.dump() + "]");
    }
    else if (type == TKeyboard::CALLBACK_QUERY)
    {
        nlohmann::json json;
        json["text"] = text;
        json["callback_data"] = value;
        this->buttons.push_back("[" + json.dump() + "]");
    }
    else
        return false;
    return true;
}

bool TKeyboard::addButton(const TKeyButtonConstructor_t &button)
{
    return addButton(button.type, button.text, button.value);
}

bool TKeyboard::addButton(const std::vector<TKeyButtonConstructor_t> &buttons)
{
    if (this->type != TKeyboard::INLINE_KEYBOARD)
        return false;
    size_t sz = buttons.size();
    if (sz == 0)
        return false;
    std::string result = "[";
    sz--;
    for (int i = 0; i < sz; i++)
    {
        if (buttons.at(i).type == TKeyboard::URL)
        {
            result += "{\"text\":\"" + buttons.at(i).text + "\",\"url\":\"" + buttons.at(i).value + "\"},";
        }
        else if (buttons.at(i).type == TKeyboard::CALLBACK_QUERY)
        {
            result += "{\"text\":\"" + buttons.at(i).text + "\",\"callback_data\":\"" + buttons.at(i).value + "\"},";
        }
        else
            return false;
    }
    if (buttons.at(sz).type == TKeyboard::URL)
    {
        result += "{\"text\":\"" + buttons.at(sz).text + "\",\"url\":\"" + buttons.at(sz).value + "\"},";
    }
    else if (buttons.at(sz).type == TKeyboard::CALLBACK_QUERY)
    {
        result += "{\"text\":\"" + buttons.at(sz).text + "\",\"callback_data\":\"" + buttons.at(sz).value + "\"}]";
    }
    else
        return false;
    this->buttons.push_back(result);
    return true;
}

TKeyboard &TKeyboard::add(const std::string &button)
{
    if (!(this->addButton(button)))
        throw std::runtime_error(Error::common(__FILE__, __LINE__, __func__, "invalid input"));
    return *this;
}

TKeyboard &TKeyboard::add(TValueType_t type, const std::string &text, const std::string &value)
{
    if (!(this->addButton(type, text, value)))
        throw std::runtime_error(Error::common(__FILE__, __LINE__, __func__, "invalid input"));
    return *this;
}

TKeyboard &TKeyboard::add(const TKeyButtonConstructor_t &button)
{
    if (!(this->addButton(button)))
        throw std::runtime_error(Error::common(__FILE__, __LINE__, __func__, "invalid input"));
    return *this;
}

TKeyboard &TKeyboard::add(const std::vector<TKeyButtonConstructor_t> &buttons)
{
    if (!(this->addButton(buttons)))
        throw std::runtime_error(Error::common(__FILE__, __LINE__, __func__, "invalid input"));
    return *this;
}

const std::string &TKeyboard::getCaption() const
{
    return this->caption;
}

std::string TKeyboard::getMarkup() const
{
    size_t sz = this->buttons.size();
    std::string result = "";
    if (sz)
    {
        result = this->type == TKeyboard::KEYBOARD ? "{\"keyboard\":[" : "{\"inline_keyboard\":[";
        sz--;
        for (int i = 0; i < sz; i++)
        {
            result += this->buttons.at(i) + ",";
        }
        result += this->buttons.at(sz) + "]}";
    }
    return result;
}

bool Telegram::apiSendKeyboard(long long targetId, const TKeyboard &keyboard)
{
    std::string replayMarkup = keyboard.getMarkup();
    if (replayMarkup.length() == 0)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "invalid keyboard!\n");
        return false;
    }
    std::string data = "{\"chat_id\":" + std::to_string(targetId) + ",\"text\":\"" + keyboard.getCaption() + "\",\"reply_markup\":" + replayMarkup + "}";
    Request req(TELEGRAM_BASE_URL, this->token, Request::SEND_MESSAGE, data);
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}

bool Telegram::apiEditInlineKeyboard(long long targetId, long long messageId, const TKeyboard &keyboard)
{
    std::string replayMarkup = keyboard.getMarkup();
    if (replayMarkup.length() == 0)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "invalid keyboard!\n");
        return false;
    }
    std::string data = "{\"chat_id\":" + std::to_string(targetId) + ",\"message_id\":" + std::to_string(messageId) + ",\"text\":\"" + keyboard.getCaption() + "\",\"reply_markup\":" + replayMarkup + "}";
    Request req(TELEGRAM_BASE_URL, this->token, Request::EDIT_MESSAGE_TEXT, data);
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}