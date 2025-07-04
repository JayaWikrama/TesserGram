#include "json-parser.hpp"
#include "type.hpp"

User::User(){
    this->isBot = false;
    this->id = 0;
}

User::~User(){

}

bool User::parse(const std::string &json){
    if (json.length() == 0) return false;
    JsonObject j(json);
    this->isBot = j["is_bot"].getString() == "true" ? true : false;
    this->id = j["id"].isAvailable() ? std::stoll(j["id"].getString()) : 0;
    this->firstName = j["first_name"].getString();
    this->lastName = j["last_name"].getString();
    this->username = j["username"].getString();
    this->languageCode = j["language_code"].getString();
    return (this->id != 0);
}

Chat::Chat(){
    this->isForum = false;
    this->type = Chat::PRIVATE;
    this->id = 0;
}

Chat::~Chat(){

}

bool Chat::parse(const std::string &json){
    if (json.length() == 0) return false;
    JsonObject j(json);
    this->isForum = j["is_forum"].getString() == "true" ? true : false;
    if (j["type"].isAvailable()){
        std::string chatType = j["type"].getString();
        if (chatType == "group") this->type = Chat::GROUP;
        else if (chatType == "supergroup") this->type == Chat::SUPERGROUP;
        else if (chatType == "channel") this->type = Chat::CHANNEL;
    }
    this->id = j["id"].isAvailable() ? std::stoll(j["id"].getString()) : 0;
    this->firstName = j["first_name"].getString();
    this->lastName = j["last_name"].getString();
    this->username = j["username"].getString();
    if (this->type != Chat::PRIVATE) this->title = j["title"].getString();
    else {
        this->title = this->firstName;
        if (this->lastName.length() > 0){
            this->title += " " + this->lastName;
        }
    }
    return (this->id != 0);
}

Media::Media(){
    this->type = Media::DOCUMENT;
    this->fileSize = 0;
}

Media::~Media(){

}

bool Media::parse(TYPE_t type, const std::string &json){
    if (json.length() == 0) return false;
    JsonObject j(json);
    this->type = type;
    this->fileSize = j["file_size"].isAvailable() ? std::stoll(j["file_size"].getString()) : 0;
    this->fileId = j["file_id"].getString();
    this->fileUniqueId = j["file_unique_id"].getString();
    this->fileName = j["file_name"].getString();
    return (this->fileId.length() != 0);
}

Message::Message(){
    this->dtime = 0;
    this->id = 0;
    this->threadId = 0;
    this->replyToMessage = nullptr;
    this->media = nullptr;
}

Message::~Message(){
    if (this->replyToMessage) delete this->replyToMessage;
    if (this->media) delete this->media;
}

bool Message::parse(const std::string &json){
    if (json.length() == 0) return false;
    JsonObject j(json);
    this->dtime = j["date"].isAvailable() ? std::stoll(j["date"].getString()) : 0;
    this->id = j["message_id"].isAvailable() ? std::stoll(j["message_id"].getString()) : 0;
    this->threadId = j["message_thread_id"].isAvailable() ? std::stoll(j["message_thread_id"].getString()) : 0;
    this->text = j["text"].getString();
    this->caption = j["caption"].getString();
    this->from.parse(j["from"].getString());
    this->chat.parse(j["chat"].getString());
    if (j["reply_to_message"].isAvailable()){
        this->replyToMessage = new Message();
        if (this->replyToMessage->parse(j["reply_to_message"].getString()) == false){
            delete this->replyToMessage;
            this->replyToMessage = nullptr;
        }
    }
    if (j["document"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::DOCUMENT, j["document"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    else if (j["photo"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::PHOTO, j["photo"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    else if (j["animation"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::ANIMATION, j["animation"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    else if (j["sticker"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::STICKER, j["sticker"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    else if (j["story"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::STORY, j["story"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    else if (j["video"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::VIDEO, j["video"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    else if (j["video_note"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::VIDEO_NOTE, j["video_note"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    else if (j["voice"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::VOICE, j["voice"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    else if (j["audio"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::AUDIO, j["audio"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    else if (j["contact"].isAvailable()){
        this->media = new Media();
        if (this->media->parse(Media::CONTACT, j["contact"].getString()) == false){
            delete this->media;
            this->media = nullptr;
        }
    }
    return (this->id != 0);
}

CallbackQuery::CallbackQuery(){
    this->id = 0;
    this->message = nullptr;
}

CallbackQuery::~CallbackQuery(){
    if (this->message) delete this->message;
}

bool CallbackQuery::parse(const std::string &json){
    if (json.length() == 0) return false;
    JsonObject j(json);
    this->id = j["id"].isAvailable() ? std::stoll(j["id"].getString()) : 0;
    this->chatInstance = j["chat_instance"].getString();
    this->data = j["data"].getString();
    this->from.parse(j["from"].getString());
    if (j["message"].isAvailable()){
        this->message = new Message();
        if (this->message->parse(j["message"].getString()) == false){
            delete this->message;
            this->message = nullptr;
        }
    }
    return (this->id != 0);
}
