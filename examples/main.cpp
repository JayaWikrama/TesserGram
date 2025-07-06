#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include "json-parser.hpp"
#include "fetch-api.hpp"
#include "telegram.hpp"
#include "debug.hpp"

Debug debug(10);

std::string readenv() {
    std::ifstream file(".env");
    if (!file.is_open()) {
        debug.log(Debug::ERROR, __PRETTY_FUNCTION__, "ENV not found!\n");
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    return ss.str();
}


bool writeBinaryFile(const std::string& filename, const std::vector<unsigned char>& data) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();

    return file.good();
}

std::string getReplay(const std::string& message){
    std::string url = "http://localhost:8000/api/v1/ask";
    FetchAPI api(url, 900, 300);
    api.enableDebug();
    api.insertHeader("Content-Type", "application/json");
    api.setBody("{\"question\":\"" + message + "\"}");
    bool success = api.post();
    if (success){
        JsonObject json(api.getPayload());
        if (json["answer"].isAvailable()){
            return json["answer"].getString();
        }
    }
    return "Mohon maaf, saya lagi tidak bisa menjawab pertanyaan. Server lagi down.";
}

void processMedia(Telegram &telegram, const std::vector<Media> &media){
    if (media.size() == 0) return;
    std::string path = telegram.apiGetMediaPath(media.at(media.size() - 1).fileId);
    debug.log(Debug::INFO, __PRETTY_FUNCTION__, "Media path of \"%s\": %s\n", media.at(media.size() - 1).fileId.c_str(), path.c_str());
    std::vector <unsigned char> mediaPayload = telegram.apiDownloadMediaByPath(path);
    writeBinaryFile(path, mediaPayload);
}

void updatesCallback(Telegram &telegram, void *ptr){
    const NodeMessage *msg = telegram.message.getMessage();
    while (msg){
        msg->display();
        if (ptr == nullptr){
            if (msg->message){
                telegram.apiSendChatAction(msg->message->chat.id, Telegram::TYPING);
                if (msg->message->text.length() > 0){
                    std::string reply = getReplay(msg->message->text);
                    telegram.apiSendMessage(msg->message->chat.id, reply);
                }
                processMedia(telegram, msg->message->media);
            }
            else {
                telegram.apiSendChatAction(msg->callbackQuery->message->chat.id, Telegram::TYPING);
                std::string reply = getReplay(msg->callbackQuery->data);
                telegram.apiSendMessage(msg->callbackQuery->message->chat.id, reply);
            }
        }
        telegram.message.dequeue();
        msg = telegram.message.getMessage();
    }
}

void webhookRoutine(Telegram &telegram, const std::string &url){
    telegram.setWebhookCallback(updatesCallback, nullptr);
    telegram.apiSetWebhook(url);
    telegram.servWebhook();
}

void getUpdatesRoutine(Telegram &telegram){
    bool isClear = false;
    unsigned short sleepPeriod = 10;
    unsigned short pcounter = 0;
    for (;;){
        if (telegram.apiGetUpdates()){
            updatesCallback(telegram, (void *) (!isClear));
            isClear = true;
            sleepPeriod = 3;
            pcounter = 0;
        }
        else if (pcounter < 4){
            pcounter++;
        }
        else if (sleepPeriod != 10) {
            sleepPeriod = 10;
        }
        sleep(sleepPeriod);
    }
}

int main(int argc, char **argv){
    std::string dotenvPayload = readenv();
    JsonObject env(dotenvPayload);
    Telegram telegram(env["bot->token"].getString());
    telegram.enableDebug();
    std::string webhookUrl = env["bot->webhook->url"].getString();
    if (telegram.apiGetMe()){
        debug.log(Debug::INFO, __PRETTY_FUNCTION__, "Telegram Bot Id: %lli\n", telegram.getId());
        debug.log(Debug::INFO, __PRETTY_FUNCTION__, "Telegram Bot Name: %s\n", telegram.getName().c_str());
        debug.log(Debug::INFO, __PRETTY_FUNCTION__, "Telegram Bot Username: %s\n", telegram.getUsername().c_str());
        if (webhookUrl.length() > 0){
            webhookRoutine(telegram, webhookUrl);
        }
        else {
            telegram.apiUnsetWebhook();
            getUpdatesRoutine(telegram);
        }
    }
    else {
        debug.log(Debug::ERROR, __PRETTY_FUNCTION__, "Failed to access telegram!\n");
    }
    return 0;
}
