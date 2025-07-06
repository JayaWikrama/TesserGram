#include "json-parser.hpp"
#include "type.hpp"

struct MediaTypeEntry {
    Media::TYPE_t type;
    const char* key;
};

static const MediaTypeEntry mediaTypes[] = {
    {Media::DOCUMENT, "document"},
    {Media::PHOTO, "photo"},
    {Media::ANIMATION, "animation"},
    {Media::STICKER, "sticker"},
    {Media::STORY, "story"},
    {Media::VIDEO, "video"},
    {Media::VIDEO_NOTE, "video_note"},
    {Media::VOICE, "voice"},
    {Media::AUDIO, "audio"},
    {Media::CONTACT, "contact"}
};

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

const std::string Media::getType(){
    for (const auto& entry : mediaTypes) {
        if (entry.type == this->type) return std::string(entry.key);
    }
    return "unknown";
}

Message::Message(){
    this->dtime = 0;
    this->id = 0;
    this->threadId = 0;
    this->replyToMessage = nullptr;
}

Message::~Message(){
    if (this->replyToMessage) delete this->replyToMessage;
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

    for (const auto& entry : mediaTypes) {
        if (j[entry.key].isAvailable()) {
            if (j[entry.key].getType() == "array"){
                int arrayLength = j[entry.key].getArraySize();
                for (int i = 0; i < arrayLength; i++) {
                    Media md;
                    if (md.parse(entry.type, j[std::string(entry.key) + "[" + std::to_string(i) + "]"].getString())){
                        this->media.push_back(md);
                    }
                }
            }
            else {
                Media md;
                if (md.parse(entry.type, j[entry.key].getString())){
                    this->media.push_back(md);
                }
            }
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
