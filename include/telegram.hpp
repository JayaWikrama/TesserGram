#ifndef __TELEGRAM_API_HPP__
#define __TELEGRAM_API_HPP__

#include <string>
#include "request.hpp"

#define TELEGRAM_BASE_URL "https://api.telegram.org"

class NodeMessage {
    public:
        time_t time;
        long long id;
        struct Sender_t {
            bool isBot;
            long long id;
            std::string name;
            std::string username;
        } sender;
        struct ChatRoom_t {
            long long id;
            std::string title;
            std::string type;
        } room;
        std::string message;
        NodeMessage *next;
        
        NodeMessage(const std::string& message);
        void display() const;
};

class Messages {
    private:
        size_t n;
        NodeMessage *first;
        NodeMessage *end;
    
    public:
        Messages();
        ~Messages();
        void enqueue(const std::string& message);
        void dequeue();
        void clear();
        const NodeMessage *getMessage() const;
};

class Telegram {
    private:
        long long id;
        long long lastUpdateId;
        std::string name;
        std::string username;
        std::string token;

        bool __apiSendMedia(long long targetId, Request::REQUEST_t type, const std::string& label, const std::string& filePath);

    public:
        Messages message;
        void *webhookCallback;
        void *webhookCallbackData;
        Telegram(const std::string &token);
        ~Telegram();
        long long getId();
        const std::string& getName() const;
        const std::string& getUsername() const;

        bool apiGetMe();
        bool apiGetUpdates();
        bool apiSendMessage(long long targetId, const std::string& message);

        bool apiSendDocument(long long targetId, const std::string& label, const std::string& filePath);
        bool apiSendPhoto(long long targetId, const std::string& label, const std::string& filePath);
        bool apiSendAudio(long long targetId, const std::string& label, const std::string& filePath);
        bool apiSendVoice(long long targetId, const std::string& label, const std::string& filePath);
        bool apiSendAnimation(long long targetId, const std::string& label, const std::string& filePath);
        bool apiSendVideo(long long targetId, const std::string& label, const std::string& filePath);

        bool apiSetWebhook(const std::string& url, const std::string& secretToken, const std::string& allowedUpdates, unsigned short maxConnection);
        bool apiSetWebhook(const std::string& url, const std::string& secretToken, const std::string& allowedUpdates);
        bool apiSetWebhook(const std::string& url, unsigned short maxConnection);
        bool apiSetWebhook(const std::string& url);
        bool apiUnsetWebhook();

        void setWebhookCallback(void (*__callback)(Telegram &, void *), void *data);
        void servWebhook();
};

#endif