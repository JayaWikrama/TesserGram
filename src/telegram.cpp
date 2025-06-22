#include <stdexcept>
#include <iostream>
#include <cstring>
#include "telegram.hpp"
#include "request.hpp"
#include "json-parser.hpp"

#define TELEGRAM_BASE_URL "https://api.telegram.org"

NodeMessage::NodeMessage(const std::string& message){
    JsonObject json(message);
    if (json["update_id"].isAvailable() == false) throw std::runtime_error("Invalid input: " + message + "!");
    this->time = static_cast<time_t>(std::stoll(json["message->date"].getString()));
    this->id = std::stoll(json["update_id"].getString());
    this->sender.isBot = (json["message->from->is_bot"].getString() == "true" ? true : false);
    this->sender.id = std::stoll(json["message->from->id"].getString());
    this->sender.name = json["message->from->first_name"].getString();
    this->sender.username = json["message->from->username"].getString();
    this->room.id = std::stoll(json["message->chat->id"].getString());
    this->room.type = json["message->chat->type"].getString();
    if (this->room.type == "private"){
        this->room.title = json["message->chat->first_name"].getString();
    }
    else {
        this->room.title = json["message->chat->title"].getString();
    }
    this->message = json["message->text"].getString();
    this->next = nullptr;
}

void NodeMessage::display() const {
    struct tm dtime;
    char dtimestr[20];
    memcpy(&dtime, localtime(&(this->time)), sizeof(dtime));
    sprintf(dtimestr,
            "%04d-%02d-%02d %02d:%02d:%02d",
            dtime.tm_year + 1900,
            dtime.tm_mon + 1,
            dtime.tm_mday,
            dtime.tm_hour,
            dtime.tm_min,
            dtime.tm_sec
        );
    dtimestr[19] = 0x00;
    std::cout << "Message ID  : " << this->id << " [" << dtimestr << "]:" << std::endl;
    std::cout << "  Sender    : " << this->sender.name << std::endl;
    std::cout << "  Room Id   : " << this->room.id << std::endl;
    std::cout << "  Room Name : " << this->room.title << std::endl;
    std::cout << "  Message   : " << this->message << std::endl;
}

void Messages::enqueue(const std::string& message){
    NodeMessage *msg = nullptr;
    try {
        msg = new NodeMessage(message);
        if (this->first == nullptr){
            this->first = msg;
            this->end = msg;
        }
        else {
            this->n++;
            this->end->next = msg;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught runtime_error: " << e.what() << std::endl;
    }
}

void Messages::dequeue(){
    if (this->first == nullptr) return;
    NodeMessage *tmp = this->first->next;
    delete this->first;
    this->first = tmp;
    this->n--;
    if (this->first == nullptr) this->end = nullptr;
}

Messages::Messages(){
    this->n = 0;
    this->first = nullptr;
    this->end = nullptr;
}

Messages::~Messages(){
    while (this->first) this->dequeue();
}

void Messages::clear(){
    while (this->first) this->dequeue();
}

const NodeMessage *Messages::getMessage() const {
    return this->first;
}

Telegram::Telegram(const std::string &token){
    this->id = 0UL;
    this->name = "";
    this->username = "";
    this->token = token;
}

Telegram::~Telegram(){

}

long long Telegram::getId(){
    return this->id;
}

const std::string& Telegram::getName() const{
    return this->name;
}

const std::string& Telegram::getUsername() const{
    return this->username;
}

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

bool Telegram::apigetUpdates(){
    Request req(TELEGRAM_BASE_URL, this->token, Request::UPDATES);
    if (req.isSuccess()){
        JsonObject json(req.getResponse());
        int arrayLength = json["result"].getArraySize();
        for (int i = 0; i < arrayLength; i++) {
            this->message.enqueue(json["result[" + std::to_string(i) + "]"].getString());
        }
        return true;
    }
    return false;
}

bool Telegram::apiSendMessage(long long targetId, const std::string& message){
    std::string data = "{\"chat_id\":" + std::to_string(targetId) + ",\"text\":\"" + message + "\"}";
    Request req(TELEGRAM_BASE_URL, this->token, Request::SEND_MESSAGE, data);
    if (req.isSuccess()){
        std::cout << "success" << std::endl;
        return true;
    }
    return false;
}