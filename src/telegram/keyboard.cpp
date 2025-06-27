#include <stdexcept>
#include <iostream>
#include <cstring>
#include "telegram.hpp"
#include "request.hpp"
#include "json-parser.hpp"

TKeyboard::TKeyboard(TKeyboard::TKeyType_t type, const std::string &caption){
    this->type = type;
    this->caption = caption;
}

TKeyboard::~TKeyboard(){
    // do nothing
}

bool TKeyboard::addButton(const std::string &button){
    if (this->type != TKeyboard::KEYBOARD) return false;
    JsonObject json(button);
    if (json.getType() != "array") return false;
    this->buttons.push_back(button);
    return true;
}

bool TKeyboard::addButton(TKeyboard::TValueType_t type, const std::string &text, const std::string &value){
    if (this->type != TKeyboard::INLINE_KEYBOARD) return false;
    if (type == TKeyboard::URL){
        this->buttons.push_back("[{\"text\":\"" + text + "\",\"url\":\"" + value + "\"}]");
    }
    else if (type == TKeyboard::CALLBACK_QUERY){
        this->buttons.push_back("[{\"text\":\"" + text + "\",\"callback_data\":\"" + value + "\"}]");
    }
    else return false;
    return true;
}

const std::string& TKeyboard::getCaption() const {
    return this->caption;
}

std::string TKeyboard::getMarkup() const {
    size_t sz = this->buttons.size();
    std::string result = "";
    if (sz){
        result = this->type == TKeyboard::KEYBOARD ? "{\"keyboard\":[" : "{\"inline_keyboard\":[";
        sz--;
        int i = 0;
        for (i = 0; i < sz; i++){
            result += this->buttons.at(i) + ",";
        }
        result += this->buttons.at(i) + "]}";
    }
    return result;
}

bool Telegram::apiSendKeyboard(long long targetId, const TKeyboard &keyboard){
    std::string replayMarkup = keyboard.getMarkup();
    if (replayMarkup.length() == 0){
        std::cout << "invalid keyboard!" << std::endl;
        return false;
    }
    std::string data = "{\"chat_id\":" + std::to_string(targetId) + ",\"text\":\"" + keyboard.getCaption() + "\",\"reply_markup\":" + replayMarkup + "}";
    Request req(TELEGRAM_BASE_URL, this->token, Request::SEND_MESSAGE, data);
    if (req.isSuccess()){
        std::cout << "success" << std::endl;
        return true;
    }
    return false;
}