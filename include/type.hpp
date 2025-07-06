#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#include <string>
#include <vector>
#include <ctime>

class User {
    public:
        bool isBot;
        long long id;
        std::string firstName;
        std::string lastName;
        std::string username;
        std::string languageCode;

        User();
        ~User();
        bool parse(const std::string &json);
};

class Chat {
    public:
        typedef enum _TYPE_t {
            PRIVATE = 0,
            GROUP,
            SUPERGROUP,
            CHANNEL
        } TYPE_t;

        bool isForum;
        TYPE_t type;
        long long id;
        std::string title;
        std::string firstName;
        std::string lastName;
        std::string username;

        Chat();
        ~Chat();
        bool parse(const std::string &json);
};

class Media {
    public:
        typedef enum _TYPE_t {
            DOCUMENT = 0,
            PHOTO,
            ANIMATION,
            STICKER,
            STORY,
            VIDEO,
            VIDEO_NOTE,
            VOICE,
            AUDIO,
            CONTACT
        } TYPE_t;

        TYPE_t type;
        long long fileSize;
        std::string fileId;
        std::string fileUniqueId;
        std::string fileName;

        Media();
        ~Media();
        bool parse(TYPE_t type, const std::string &json);
        const std::string getType();
};

class Message {
    public:
        time_t dtime;
        long long id;
        long long threadId;
        std::string text;
        std::string caption;
        User from;
        Chat chat;
        std::vector <Media> media;
        Message *replyToMessage;

        Message();
        ~Message();
        bool parse(const std::string &json);
};

class CallbackQuery {
    public:
        long long id;
        std::string chatInstance;
        std::string data;
        User from;
        Message *message;

        CallbackQuery();
        ~CallbackQuery();
        bool parse(const std::string &json);
};

#endif