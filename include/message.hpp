#ifndef __UPDATES_HPP__
#define __UPDATES_HPP__

#include <string>
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
        std::string username;
        std::string firstName;
        std::string lastName;
        std::string username;

        Chat();
}

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
        std::strimg fileName;

        Media();
}

class Message {
    public:
        time_t dtime;
        long long id;
        long long threadId;
        std::string text;
        std::string caption;
        User from;
        Chat chat;
        Message *message;
        Media *media;

        Message();
        ~Message();
}

#endif