#include <stdexcept>
#include <iostream>
#include <cstring>
#include "telegram.hpp"
#include "json-parser.hpp"

NodeMessage::NodeMessage(const std::string& message){
    JsonObject json(message);
    JsonObject jmessage;
    if (json["update_id"].isAvailable() == false) throw std::runtime_error("Invalid input: " + message + "!");
    this->updateId = std::stoll(json["update_id"].getString());
    if (json["callback_query"].isAvailable()){
        this->callbackId = std::stoll(json["callback_query->id"].getString());
        this->message = json["callback_query->data"].getString();
        this->type = NodeMessage::CALLBACK_QUERY;
        jmessage.parse(json["callback_query->message"].getString());
    }
    else {
        this->message = json["message->text"].getString();
        this->type = NodeMessage::MESSAGE;
        jmessage.parse(json["message"].getString());
    }
    this->id = std::stoll(jmessage["message_id"].getString());
    this->time = jmessage["date"].isAvailable() ? static_cast<time_t>(std::stoll(jmessage["date"].getString())) : 0;
    this->sender.id = jmessage["from->id"].isAvailable() ? std::stoll(jmessage["from->id"].getString()) : 0;
    this->room.id = jmessage["chat->id"].isAvailable() ? std::stoll(jmessage["chat->id"].getString()) : 0;
    this->sender.isBot = (jmessage["from->is_bot"].getString() == "true" ? true : false);
    this->sender.name = jmessage["from->first_name"].getString();
    this->sender.username = jmessage["from->username"].getString();
    this->room.type = jmessage["chat->type"].getString();
    if (this->room.type == "private"){
        this->room.title = jmessage["chat->first_name"].getString();
    }
    else {
        this->room.title = jmessage["chat->title"].getString();
    }
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
    std::cout << "Update ID   : " << this->updateId << " [" << dtimestr << "]:" << std::endl;
    std::cout << "  Type      : " << (this->type == NodeMessage::MESSAGE ? "message" : "callback_query") << std::endl;
    std::cout << "  id        : " << this->id << std::endl;
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
            this->n++;
        }
        else {
            this->n++;
            this->end->next = msg;
        }
        this->end = msg;
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
    this->id = 0;
    this->lastUpdateId = 0;
    this->name = "";
    this->username = "";
    this->token = token;
    this->webhookCallback = nullptr;
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