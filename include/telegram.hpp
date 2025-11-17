#ifndef __TELEGRAM_API_HPP__
#define __TELEGRAM_API_HPP__

#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <functional>

#include "type.hpp"
#include "node-message.hpp"
#include "keyboard.hpp"
#include "polling-controller.hpp"

#define TELEGRAM_BASE_URL "https://api.telegram.org"

class Telegram
{
public:
    Telegram();
    Telegram(const std::string &token);
    ~Telegram();

    void setToken(const std::string &token);

    long long getId() const;
    const std::string &getName() const;
    const std::string &getUsername() const;
    void info() const;

    void clearUpdates();
    bool getUpdates(std::function<void(Telegram &, const NodeMessage &)> handler);
    void getUpdatesPoll(std::function<void(Telegram &, const NodeMessage &)> handler);

    bool apiGetMe();
    bool apiGetUpdates();
    bool apiSendMessage(long long targetId, const std::string &message);
    bool apiEditMessageText(long long targetId, long long messageId, const std::string &message);
    bool apiSendChatAction(long long targetId, Chat::Action action);

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
    void setWebhookCallback(std::function<void(Telegram &, const NodeMessage &)> handler);
    void execWebhookCallback();
    void servWebhook();

    bool apiSendKeyboard(long long targetId, const TKeyboard &keyboard);
    bool apiEditInlineKeyboard(long long targetId, long long messageId, const TKeyboard &keyboard);

    bool parseGetUpdatesResponse(const std::string &buffer);

private:
    long long id;
    long long lastUpdateId;
    std::string name;
    std::string username;
    std::string token;

    std::function<void(Telegram &, const NodeMessage &)> webhookCallback;
    std::deque<NodeMessage> messages;

    PollingController controller;

    mutable std::mutex mutex;

    bool __apiSendMedia(long long targetId, Media::Type type, const std::string &label, const std::string &filePath);
    bool __parseGetUpdatesResponse(const std::string &buffer);
};

#endif