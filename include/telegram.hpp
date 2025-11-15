#ifndef __TELEGRAM_API_HPP__
#define __TELEGRAM_API_HPP__

#include <string>
#include <vector>
#include <deque>

#include "type.hpp"
#include "function.hpp"
#include "node-message.hpp"
#include "keyboard.hpp"

#define TELEGRAM_BASE_URL "https://api.telegram.org"

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

    Function webhookCallback;
    std::deque<NodeMessage> messages;

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
    void setWebhookCallback(const Function &func);
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