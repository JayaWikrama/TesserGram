#include <stdexcept>
#include <iostream>
#include <cstring>
#include "telegram.hpp"
#include "request.hpp"
#include "utils/include/debug.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"

TKeyboard::TKeyButton::TKeyButton(TKeyboard::TKeyButton::Type type, const std::string &text, const std::string &value) : type(type), text(text), value(value) {}

TKeyboard::TKeyButton::~TKeyButton() {}

TKeyboard::TKeyButton::Type TKeyboard::TKeyButton::getType() const
{
    return this->type;
}

const std::string &TKeyboard::TKeyButton::getText() const
{
    return this->text;
}

const std::string &TKeyboard::TKeyButton::getValue() const
{
    return this->value;
}

TKeyboard::TKeyboard(TKeyboard::Type type, const std::string &caption)
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
    if (this->type != TKeyboard::Type::KEYBOARD)
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

bool TKeyboard::addButton(TKeyboard::TKeyButton::Type type, const std::string &text, const std::string &value)
{
    if (this->type != TKeyboard::Type::INLINE_KEYBOARD)
        return false;
    if (type == TKeyboard::TKeyButton::Type::URL)
    {
        nlohmann::json json;
        json["text"] = text;
        json["url"] = value;
        this->buttons.push_back("[" + json.dump() + "]");
    }
    else if (type == TKeyboard::TKeyButton::Type::CALLBACK_QUERY)
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

bool TKeyboard::addButton(const TKeyboard::TKeyButton &button)
{
    return addButton(button.getType(), button.getText(), button.getValue());
}

bool TKeyboard::addButton(const std::vector<TKeyboard::TKeyButton> &buttons)
{
    if (this->type != TKeyboard::Type::INLINE_KEYBOARD)
        return false;

    if (buttons.size() == 0)
        return false;

    nlohmann::json arr = nlohmann::json::array();
    for (const TKeyboard::TKeyButton &button : buttons)
    {
        nlohmann::json jsonButton;
        if (button.getType() == TKeyboard::TKeyButton::Type::URL)
        {
            jsonButton = {
                {"text", button.getText()},
                {"url", button.getValue()}};
        }
        else if (button.getType() == TKeyboard::TKeyButton::Type::CALLBACK_QUERY)
        {
            jsonButton = {
                {"text", button.getText()},
                {"callback_data", button.getValue()}};
        }
        else
            return false;
        arr.push_back(jsonButton);
    }

    this->buttons.push_back(arr.dump());
    return true;
}

TKeyboard &TKeyboard::add(const std::string &button)
{
    if (!(this->addButton(button)))
        throw std::runtime_error(Error::common(__FILE__, __LINE__, __func__, "invalid input"));
    return *this;
}

TKeyboard &TKeyboard::add(TKeyboard::TKeyButton::Type type, const std::string &text, const std::string &value)
{
    if (!(this->addButton(type, text, value)))
        throw std::runtime_error(Error::common(__FILE__, __LINE__, __func__, "invalid input"));
    return *this;
}

TKeyboard &TKeyboard::add(const TKeyboard::TKeyButton &button)
{
    if (!(this->addButton(button)))
        throw std::runtime_error(Error::common(__FILE__, __LINE__, __func__, "invalid input"));
    return *this;
}

TKeyboard &TKeyboard::add(const std::vector<TKeyboard::TKeyButton> &buttons)
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
    nlohmann::json arr = nlohmann::json::array();

    try
    {
        for (const std::string &button : this->buttons)
        {
            nlohmann::json jsonButton = nlohmann::json::parse(button);
            arr.push_back(jsonButton);
        }
    }
    catch (const std::exception &e)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "parse button failed\n");
        return "";
    }

    nlohmann::json result;
    if (this->type == TKeyboard::Type::KEYBOARD)
        result["keyboard"] = arr;
    else
        result["inline_keyboard"] = arr;

    return result.dump();
}

bool Telegram::apiSendKeyboard(long long targetId, const TKeyboard &keyboard)
{
    std::string replayMarkup = keyboard.getMarkup();
    if (replayMarkup.empty())
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "invalid keyboard!\n");
        return false;
    }

    nlohmann::json json = {
        {"chat_id", targetId},
        {"text", keyboard.getCaption()},
        {"reply_markup", nlohmann::json::parse(replayMarkup)}};

    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::SEND_MESSAGE, json.dump());
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
    if (replayMarkup.empty())
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "invalid keyboard!\n");
        return false;
    }

    nlohmann::json json = {
        {"chat_id", targetId},
        {"message_id", messageId},
        {"text", keyboard.getCaption()},
        {"reply_markup", nlohmann::json::parse(replayMarkup)}};

    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::EDIT_MESSAGE_TEXT, json.dump());
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}