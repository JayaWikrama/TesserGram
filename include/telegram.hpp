#ifndef __TELEGRAM_API_HPP__
#define __TELEGRAM_API_HPP__

#include <string>

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

    public:
        Messages message;
        Telegram(const std::string &token);
        ~Telegram();
        long long getId();
        const std::string& getName() const;
        const std::string& getUsername() const;
        bool apiGetMe();
        bool apigetUpdates();
        bool apiSendMessage(long long targetId, const std::string& message);
};

#endif