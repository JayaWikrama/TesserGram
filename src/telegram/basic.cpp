#include <stdexcept>
#include <iostream>
#include <cstring>
#include "telegram.hpp"
#include "request.hpp"
#include "json-parser.hpp"

static const std::string ChatActionStr[] = {
    "typing",
    "upload_photo",
    "record_video",
    "upload_video",
    "record_voice",
    "upload_voice",
    "upload_document"
};

bool Telegram::apiGetMe(){
    Request req(TELEGRAM_BASE_URL, this->token, Request::CONFIG);
    if (req.isSuccess()){
        JsonObject json(req.getResponse());
        if (json["result->id"].isAvailable()){
            this->id = std::stoll(json["result->id"].getString());
            this->name = json["result->first_name"].getString();
            this->username = json["result->username"].getString();
            return true;
        }
    }
    return false;
}

bool Telegram::apiGetUpdates(){
    if (this->lastUpdateId > 0){
        std::string data = "{\"offset\":" + std::to_string(this->lastUpdateId + 1) + "}";
        Request req(TELEGRAM_BASE_URL, this->token, Request::UPDATES, data);
        if (req.isSuccess()){
            JsonObject json(req.getResponse());
            int arrayLength = json["result"].getArraySize();
            long long updateId = 0;
            for (int i = 0; i < arrayLength; i++) {
                this->message.enqueue(json["result[" + std::to_string(i) + "]"].getString());
                updateId = std::stoll(json["result[" + std::to_string(i) + "]->update_id"].getString());
                if (this->lastUpdateId < updateId) this->lastUpdateId = updateId;
            }
            return true;
        }
    }
    else {
        Request req(TELEGRAM_BASE_URL, this->token, Request::UPDATES);
        if (req.isSuccess()){
            JsonObject json(req.getResponse());
            int arrayLength = json["result"].getArraySize();
            long long updateId = 0;
            for (int i = 0; i < arrayLength; i++) {
                this->message.enqueue(json["result[" + std::to_string(i) + "]"].getString());
                updateId = std::stoll(json["result[" + std::to_string(i) + "]->update_id"].getString());
                if (this->lastUpdateId < updateId) this->lastUpdateId = updateId;
            }
            return true;
        }
    }
    return false;
}

bool Telegram::apiSendMessage(long long targetId, const std::string& message){
    JsonBuilder json;
    json["chat_id"] = targetId;
    json["text"] = message;
    Request req(TELEGRAM_BASE_URL, this->token, Request::SEND_MESSAGE, json.dump());
    if (req.isSuccess()){
        std::cout << "success" << std::endl;
        return true;
    }
    return false;
}

bool Telegram::apiSendChatAction(long long targetId, Telegram::ChatAction_t action){
    std::string data = "{\"chat_id\":" + std::to_string(targetId) + ",\"action\":\"" + ChatActionStr[action] + "\"}";
    Request req(TELEGRAM_BASE_URL, this->token, Request::SEND_CHAT_ACTION, data);
    if (req.isSuccess()){
        std::cout << "success" << std::endl;
        return true;
    }
    return false;
}