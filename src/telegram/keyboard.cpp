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

TKeyboard::TKeyboard(TKeyboard::Type type, const std::string &caption) : commonButtons(), inlineButtons()
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

    this->commonButtons.emplace_back();
    this->commonButtons.back().push_back(button);

    return true;
}

bool TKeyboard::addButton(const std::vector<std::string> &buttons)
{
    if (this->type != TKeyboard::Type::KEYBOARD)
        return false;

    if (buttons.empty())
        return false;

    this->commonButtons.push_back(buttons);

    return true;
}

bool TKeyboard::addButton(TKeyboard::TKeyButton::Type type, const std::string &text, const std::string &value)
{
    if (this->type != TKeyboard::Type::INLINE_KEYBOARD)
        return false;

    this->inlineButtons.emplace_back();
    this->inlineButtons.back().emplace_back(type, text, value);
    return true;
}

bool TKeyboard::addButton(const TKeyboard::TKeyButton &button)
{
    if (this->type != TKeyboard::Type::INLINE_KEYBOARD)
        return false;

    this->inlineButtons.emplace_back();
    this->inlineButtons.back().push_back(button);
    return true;
}

bool TKeyboard::addButton(const std::vector<TKeyboard::TKeyButton> &buttons)
{
    if (this->type != TKeyboard::Type::INLINE_KEYBOARD)
        return false;

    if (buttons.empty())
        return false;

    this->inlineButtons.push_back(buttons);

    return true;
}

TKeyboard &TKeyboard::add(const std::string &button)
{
    if (!(this->addButton(button)))
        throw std::runtime_error(Error::common(__FILE__, __LINE__, __func__, "invalid input"));
    return *this;
}

TKeyboard &TKeyboard::add(const std::vector<std::string> &buttons)
{
    if (!(this->addButton(buttons)))
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

const std::vector<std::vector<std::string>> &TKeyboard::getCommonButton() const
{
    return this->commonButtons;
}

const std::vector<std::vector<TKeyboard::TKeyButton>> &TKeyboard::getInlineButton() const
{
    return this->inlineButtons;
}

bool Telegram::apiSendKeyboard(long long targetId, const TKeyboard &keyboard)
{
    std::vector<std::vector<std::string>> commonButtons = keyboard.getCommonButton();
    std::vector<std::vector<TKeyboard::TKeyButton>> inlineButtons = keyboard.getInlineButton();

    nlohmann::json jsonButton = nlohmann::json::array();
    nlohmann::json jsonKeyboard;

    if (commonButtons.empty() == false)
    {
        for (const std::vector<std::string> &row : commonButtons)
        {
            nlohmann::json arr = nlohmann::json::array();
            for (const std::string &button : row)
            {
                arr.push_back(button);
            }
            jsonButton.push_back(arr);
        }
        jsonKeyboard["keyboard"] = jsonButton;
    }
    else if (inlineButtons.empty() == false)
    {
        for (const std::vector<TKeyboard::TKeyButton> &row : inlineButtons)
        {
            nlohmann::json arr = nlohmann::json::array();
            for (const TKeyboard::TKeyButton &button : row)
            {
                nlohmann::json j = {
                    {"text", button.getText()},
                    {(button.getType() == TKeyboard::TKeyButton::Type::URL ? "url" : "callback_data"), button.getValue()}};
                arr.push_back(j);
            }
            jsonButton.push_back(arr);
        }
        jsonKeyboard["inline_keyboard"] = jsonButton;
    }
    else
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "invalid keyboard!\n");
        return false;
    }

    nlohmann::json json = {
        {"chat_id", targetId},
        {"text", keyboard.getCaption()},
        {"reply_markup", jsonKeyboard}};

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
    std::vector<std::vector<TKeyboard::TKeyButton>> buttons = keyboard.getInlineButton();

    if (buttons.empty())
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "invalid keyboard!\n");
        return false;
    }

    nlohmann::json jsonButton = nlohmann::json::array();

    for (const std::vector<TKeyboard::TKeyButton> &row : buttons)
    {
        nlohmann::json arr = nlohmann::json::array();
        for (const TKeyboard::TKeyButton &button : row)
        {
            nlohmann::json j = {
                {"text", button.getText()},
                {(button.getType() == TKeyboard::TKeyButton::Type::URL ? "url" : "callback_data"), button.getValue()}};
            arr.push_back(j);
        }
        jsonButton.push_back(arr);
    }

    nlohmann::json jsonKeyboard = {
        {"keyboard", jsonButton}};

    nlohmann::json json = {
        {"chat_id", targetId},
        {"message_id", messageId},
        {"text", keyboard.getCaption()},
        {"reply_markup", jsonKeyboard}};

    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::EDIT_MESSAGE_TEXT, json.dump());
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}