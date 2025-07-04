#include <stdexcept>
#include <iostream>
#include <cstring>
#include "debug.hpp"
#include "json-parser.hpp"
#include "telegram.hpp"

NodeMessage::NodeMessage(const std::string& message){
    JsonObject json(message);
    JsonObject jmessage;
    this->callbackQuery = nullptr;
    this->message = nullptr;
    if (json["update_id"].isAvailable() == false) throw std::runtime_error("Invalid input: " + message + "!");
    this->updateId = std::stoll(json["update_id"].getString());
    if (json["callback_query"].isAvailable()){
        this->callbackQuery = new CallbackQuery();
        if (this->callbackQuery->parse(json["callback_query"].getString()) == false){
            delete this->callbackQuery;
            this->callbackQuery = nullptr;
        }
    }
    else if (json["message"].isAvailable()){
        this->message = new Message();
        if (this->message->parse(json["message"].getString()) == false){
            delete this->message;
            this->message = nullptr;
        }
    }
    if (this->message == nullptr && this->callbackQuery == nullptr){
        throw std::runtime_error("Unknown type: " + message + "!");
    }
    this->next = nullptr;
}

void NodeMessage::display() const {
    struct tm dtime;
    char dtimestr[64];
    if (this->callbackQuery){
        time_t ctime = time(nullptr);
        memcpy(&dtime, localtime(&ctime), sizeof(dtime));
    }
    else {
        memcpy(&dtime, localtime(&(this->message->dtime)), sizeof(dtime));
    }
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
    Debug debug(0);
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "Update ID   : %lli [%s]\n", this->updateId, dtimestr);
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Type      : %s\n", (this->callbackQuery == nullptr ? "message" : "callback_query"));
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  id        : %lli\n", (this->message ? this->message->id : this->callbackQuery->id));
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Sender    : %s\n", (this->message ? this->message->from.username.c_str() : this->callbackQuery->message->from.username.c_str()));
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Room Id   : %lli\n", (this->message ? this->message->chat.id : this->callbackQuery->message->chat.id));
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Room Name : %s\n", (this->message ? this->message->chat.title.c_str() : this->callbackQuery->message->chat.title.c_str()));
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "  Message   : %s\n", (this->message ? this->message->text.c_str() : this->callbackQuery->data.c_str()));
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
        Debug debug(0);
        debug.log(Debug::ERROR, __PRETTY_FUNCTION__, "Caught runtime_error: %s\n", e.what());
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
    this->debug = nullptr;
}

Telegram::~Telegram(){
    if (this->debug){
        Debug *dbg = (Debug *) this->debug;
        delete (dbg);
    }
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

void Telegram::enableDebug(){
    this->debug = (Debug_t) new Debug(0);
}

void Telegram::enableDebug(const std::string &confidential){
    Debug *dbg = new Debug(100);
    dbg->setConfidential(confidential);
    this->debug = (Debug_t) dbg;
}

void Telegram::enableDebug(const std::vector <std::string> &confidential){
    Debug *dbg = new Debug(100);
    for (int i = 0; i < confidential.size(); i++)  dbg->setConfidential(confidential.at(i));
    this->debug = (Debug_t) dbg;
}

void Telegram::disableDebug(){
    Debug *dbg = (Debug *) this->debug;
    if (dbg == nullptr) return;
    delete (dbg);
    this->debug = nullptr;
}

bool Telegram::isDebugEnable(){
  return (this->debug != nullptr);
}