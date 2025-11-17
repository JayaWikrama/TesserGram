#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include "fetch-api.hpp"
#include "telegram.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"
#include "utils/include/debug.hpp"

class MyBot
{
private:
    std::string webhook;
    Telegram telegram;

    bool writeFile(const std::string &filename, const std::vector<unsigned char> &data)
    {
        std::ofstream file(filename, std::ios::out | std::ios::binary);
        if (!file)
        {
            return false;
        }

        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();

        return file.good();
    }

    void processMedia(Telegram &telegram, const std::vector<Media> &media)
    {
        if (media.size() == 0)
            return;
        std::string path = telegram.apiGetMediaPath(media.at(media.size() - 1).fileId);
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Media path of \"%s\": %s\n", media.at(media.size() - 1).fileId.c_str(), path.c_str());
        std::vector<unsigned char> mediaPayload = telegram.apiDownloadMediaByPath(path);
        this->writeFile(path, mediaPayload);
    }

    std::string getReplay(const std::string &message)
    {
        std::string url = "http://localhost:8000/api/v1/ask";
        FetchAPI api(url, 900, 300);
        api.insertHeader("Content-Type", "application/json");
        api.setBody("{\"question\":\"" + message + "\"}");
        bool success = api.post();
        if (success)
        {
            try
            {
                nlohmann::json json = nlohmann::json::parse(api.getPayload());
                JSONValidator jvalidator(__FILE__, __LINE__, __func__);
                return jvalidator.get<std::string>(json, "answer");
            }
            catch (const std::exception &e)
            {
                Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "parse answer failed: %e\n", e.what());
            }
        }
        return "Mohon maaf, saya lagi tidak bisa menjawab pertanyaan. Server lagi down.";
    }

    void handler(Telegram &telegram, const NodeMessage &message)
    {
        message.display();
        message
            .processMessage(
                [&](const Message &m)
                {
                    telegram.apiSendChatAction(m.chat.id, Chat::Action::TYPING);
                    if (m.text.length() > 0)
                    {
                        std::string reply = this->getReplay(m.text);
                        telegram.apiSendMessage(m.chat.id, reply);
                    }
                    this->processMedia(telegram, m.media);
                })
            .processCallbackQuery(
                [&](const CallbackQuery &c)
                {
                    telegram.apiSendChatAction(c.message->chat.id, Chat::Action::TYPING);
                    std::string reply = this->getReplay(c.data);
                    telegram.apiSendMessage(c.message->chat.id, reply);
                });
    }

public:
    MyBot(const std::string &configFile) : webhook(), telegram()
    {
        nlohmann::json env = nlohmann::json::parse(std::ifstream(configFile));

        this->telegram.setToken(env["bot"]["token"].get<std::string>());

        if (env["bot"].contains("webhook"))
        {
            if (env["bot"]["webhook"].contains("url"))
            {
                this->webhook = env["bot"]["webhook"]["url"].get<std::string>();
            }
        }
    }

    ~MyBot() {}

    void start()
    {
        if (this->telegram.apiGetMe())
        {
            this->telegram.info();
            if (this->webhook.empty())
            {
                telegram.apiUnsetWebhook();

                for (;;)
                {
                    telegram.getUpdatesPoll(
                        [this](Telegram &telegram, const NodeMessage &message)
                        {
                            this->handler(telegram, message);
                        });
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
            else
            {
                telegram.setWebhookCallback(
                    [this](Telegram &telegram, const NodeMessage &message)
                    {
                        this->handler(telegram, message);
                    });
                telegram.apiSetWebhook(this->webhook);
                telegram.servWebhook();
            }
        }
        else
        {
            Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "Failed to access telegram!\n");
        }
    }
};

int main(int argc, char **argv)
{
    MyBot bot(".env");
    bot.start();
    return 0;
}
