#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#include <string>
#include <vector>
#include <ctime>
#include <memory>
#include <functional>
#include "utils/include/nlohmann/json_fwd.hpp"

class User
{
public:
    bool isBot;
    long long id;
    std::string firstName;
    std::string lastName;
    std::string username;
    std::string languageCode;

    User();
    ~User();
    bool empty() const;
    bool parse(const nlohmann::json &json);
    void reset();
};

class Chat
{
public:
    enum class Action : uint8_t
    {
        TYPING = 0x00,
        UPLOAD_PHOTO = 0x01,
        RECORD_VIDEO = 0x02,
        UPLOAD_VIDEO = 0x03,
        RECORD_VOICE = 0x04,
        UPLOAD_VOICE = 0x05,
        UPLOAD_DOCUMENT = 0x06
    };

    enum class Type : uint8_t
    {
        PRIVATE = 0x00,
        GROUP = 0x01,
        SUPERGROUP = 0x02,
        CHANNEL = 0x03
    };

    bool isForum;
    Type type;
    long long id;
    std::string title;
    std::string firstName;
    std::string lastName;
    std::string username;

    Chat();
    ~Chat();
    bool empty() const;
    bool parse(const nlohmann::json &json);
    void reset();

    static const std::string &actionToString(const Chat::Action &action);
};

class Media
{
public:
    enum class Type : uint8_t
    {
        DOCUMENT = 0x00,
        PHOTO = 0x01,
        ANIMATION = 0x02,
        STICKER = 0x03,
        STORY = 0x04,
        VIDEO = 0x05,
        VIDEO_NOTE = 0x06,
        VOICE = 0x07,
        AUDIO = 0x08,
        CONTACT = 0x09
    };

    Type type;
    long long fileSize;
    std::string fileId;
    std::string fileUniqueId;
    std::string fileName;

    Media();
    ~Media();
    bool empty() const;
    bool parse(Type type, const nlohmann::json &json);
    void reset();
    const std::string getType() const;

    static const std::string &typeToString(const Type &type);
    static void typeIteration(std::function<void(const Type &, const std::string &)> handler);
};

class Message
{
public:
    time_t dtime;
    long long id;
    long long threadId;
    std::string text;
    std::string caption;
    User from;
    Chat chat;
    std::vector<Media> media;
    std::unique_ptr<Message> replyToMessage;

    Message();
    ~Message();
    bool empty() const;
    bool parse(const nlohmann::json &json);
    void reset();
};

class CallbackQuery
{
public:
    long long id;
    std::string chatInstance;
    std::string data;
    User from;
    std::unique_ptr<Message> message;

    CallbackQuery();
    ~CallbackQuery();
    bool empty() const;
    bool parse(const nlohmann::json &json);
    void reset();
};

#endif