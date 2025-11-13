#ifndef __TELEGRAM_API_HPP__
#define __TELEGRAM_API_HPP__

#include <string>
#include <vector>
#include "type.hpp"

#define TELEGRAM_BASE_URL "https://api.telegram.org"

class TKeyboard
{
public:
    typedef enum _TKeyType_t
    {
        KEYBOARD = 0,
        INLINE_KEYBOARD = 1
    } TKeyType_t;

    typedef enum _TValueType_t
    {
        COMMON = 0,
        URL = 1,
        CALLBACK_QUERY = 2
    } TValueType_t;

    typedef struct _TKeyButtonConstructor_t
    {
        TValueType_t type;
        std::string text;
        std::string value;
    } TKeyButtonConstructor_t;

    TKeyboard(TKeyType_t type, const std::string &caption);
    ~TKeyboard();
    bool addButton(const std::string &button);
    bool addButton(TValueType_t type, const std::string &text, const std::string &value);
    bool addButton(const TKeyButtonConstructor_t &button);
    bool addButton(const std::vector<TKeyButtonConstructor_t> &buttons);
    const std::string &getCaption() const;
    std::string getMarkup() const;

private:
    TKeyType_t type;
    std::string caption;
    std::vector<std::string> buttons;
};

class NodeMessage
{
public:
    long long updateId;
    CallbackQuery *callbackQuery;
    Message *message;
    NodeMessage *next;

    NodeMessage(const std::string &message);
    void display() const;
};

class Messages
{
private:
    size_t n;
    NodeMessage *first;
    NodeMessage *end;

public:
    Messages();
    ~Messages();
    void enqueue(const std::string &message);
    void dequeue();
    void clear();
    const NodeMessage *getMessage() const;
};

class Telegram
{
public:
    typedef enum _ChatAction_t
    {
        TYPING = 0,
        UPLOAD_PHOTO,
        RECORD_VIDEO,
        UPLOAD_VIDEO,
        RECORD_VOICE,
        UPLOAD_VOICE,
        UPLOAD_DOCUMENT
    } ChatAction_t;

    typedef enum _MediaType_t
    {
        PHOTO = 0,
        AUDIO,
        VOICE,
        ANIMATION,
        VIDEO,
        DOCUMENT
    } MediaType_t;

    Messages message;
    void *webhookCallback;
    void *webhookCallbackData;
    Telegram(const std::string &token);
    ~Telegram();
    long long getId();
    const std::string &getName() const;
    const std::string &getUsername() const;

    bool apiGetMe();
    bool apiGetUpdates();
    bool apiSendMessage(long long targetId, const std::string &message);
    bool apiEditMessageText(long long targetId, long long messageId, const std::string &message);
    bool apiSendChatAction(long long targetId, ChatAction_t action);

    bool apiSendDocument(long long targetId, const std::string &label, const std::string &filePath);
    bool apiSendPhoto(long long targetId, const std::string &label, const std::string &filePath);
    bool apiSendAudio(long long targetId, const std::string &label, const std::string &filePath);
    bool apiSendVoice(long long targetId, const std::string &label, const std::string &filePath);
    bool apiSendAnimation(long long targetId, const std::string &label, const std::string &filePath);
    bool apiSendVideo(long long targetId, const std::string &label, const std::string &filePath);
    std::string apiGetMediaPath(const std::string &fileId);
    std::vector<unsigned char> apiDownloadMediaById(const std::string &fileId);
    std::vector<unsigned char> apiDownloadMediaByPath(const std::string &mediaPath);

    bool apiSetWebhook(const std::string &url, const std::string &secretToken, const std::string &allowedUpdates, unsigned short maxConnection);
    bool apiSetWebhook(const std::string &url, const std::string &secretToken, const std::string &allowedUpdates);
    bool apiSetWebhook(const std::string &url, unsigned short maxConnection);
    bool apiSetWebhook(const std::string &url);
    bool apiUnsetWebhook();
    void setWebhookCallback(void (*__callback)(Telegram &, void *), void *data);
    void servWebhook();

    bool apiSendKeyboard(long long targetId, const TKeyboard &keyboard);
    bool apiEditInlineKeyboard(long long targetId, long long messageId, const TKeyboard &keyboard);

private:
    long long id;
    long long lastUpdateId;
    std::string name;
    std::string username;
    std::string token;

    bool __apiSendMedia(long long targetId, MediaType_t type, const std::string &label, const std::string &filePath);
    bool __parseGetUpdatesResponse(const std::string &buffer);
};

#endif