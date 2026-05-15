#include "telegram.hpp"
#include "request.hpp"
#include "utils/include/debug.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"

bool Telegram::apiSetWebhook(const std::string &url, const std::string &secretToken, const std::vector<std::string> &allowedUpdates, unsigned short maxConnection)
{
    nlohmann::json json;
    json["url"] = url;
    if (secretToken.length())
        json["secret_token"] = secretToken;
    if (!allowedUpdates.empty())
        json["allowed_updates"] = allowedUpdates;
    if (maxConnection > 0)
        json["max_connections"] = maxConnection;
    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::SET_WEBHOOK, json.dump());
    if (req.isSuccess())
    {

        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}

bool Telegram::apiSetWebhook(const std::string &url, const std::string &secretToken, const std::vector<std::string> &allowedUpdates)
{
    return this->apiSetWebhook(url, secretToken, allowedUpdates, 40);
}

bool Telegram::apiSetWebhook(const std::string &url, unsigned short maxConnection)
{
    return this->apiSetWebhook(url, "", {}, maxConnection);
}

bool Telegram::apiSetWebhook(const std::string &url)
{
    return this->apiSetWebhook(url, "", {}, 40);
}

bool Telegram::apiUnsetWebhook()
{
    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::UNSET_WEBHOOK, std::string("{\"drop_pending_updates\":true}"));
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}

void Telegram::setWebhookCallback(std::function<void(Telegram &, const NodeMessage &)> handler)
{
    this->webhookCallback = handler;
}

void Telegram::servWebhook()
{
    this->server.run("http://0.0.0.0:8443", this);
}

void Telegram::stopWebhook()
{
    this->server.stop();
}

void Telegram::execWebhookCallback()
{
    if (!this->webhookCallback)
        return;
    std::deque<NodeMessage> snapshot;
    {
        std::lock_guard<std::mutex> guard(this->mutex);
        snapshot.swap(this->messages);
    }
    for (const NodeMessage &message : snapshot)
    {
        this->webhookCallback(*this, message);
    }
}