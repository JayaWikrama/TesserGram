#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include "fetch-api.hpp"
#include "telegram.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"
#include "utils/include/debug.hpp"

std::string readenv()
{
    std::ifstream file(".env");
    if (!file.is_open())
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "ENV not found!\n");
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    return ss.str();
}

bool writeBinaryFile(const std::string &filename, const std::vector<unsigned char> &data)
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

void processMedia(Telegram &telegram, const std::vector<Media> &media)
{
    if (media.size() == 0)
        return;
    std::string path = telegram.apiGetMediaPath(media.at(media.size() - 1).fileId);
    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Media path of \"%s\": %s\n", media.at(media.size() - 1).fileId.c_str(), path.c_str());
    std::vector<unsigned char> mediaPayload = telegram.apiDownloadMediaByPath(path);
    writeBinaryFile(path, mediaPayload);
}

void updatesCallback(Telegram &telegram, void *ptr)
{
    for (const NodeMessage &message : telegram.messages)
    {
        message.display();
        if (ptr == nullptr)
        {
            message
                .processMessage(
                    [&](const Message &m)
                    {
                        telegram.apiSendChatAction(m.chat.id, Telegram::TYPING);
                        if (m.text.length() > 0)
                        {
                            std::string reply = getReplay(m.text);
                            telegram.apiSendMessage(m.chat.id, reply);
                        }
                        processMedia(telegram, m.media);
                    })
                .processCallbackQuery(
                    [&](const CallbackQuery &c)
                    {
                        telegram.apiSendChatAction(c.message->chat.id, Telegram::TYPING);
                        std::string reply = getReplay(c.data);
                        telegram.apiSendMessage(c.message->chat.id, reply);
                    });
        }
    }
    telegram.messages.clear();
}

void webhookRoutine(Telegram &telegram, const std::string &url)
{
    telegram.setWebhookCallback(Function(updatesCallback, nullptr));
    telegram.apiSetWebhook(url);
    telegram.servWebhook();
}

void getUpdatesRoutine(Telegram &telegram)
{
    bool isClear = false;
    unsigned short sleepPeriod = 10;
    unsigned short pcounter = 0;
    for (;;)
    {
        if (telegram.apiGetUpdates())
        {
            updatesCallback(telegram, (void *)(!isClear));
            isClear = true;
            sleepPeriod = 3;
            pcounter = 0;
        }
        else if (pcounter < 4)
        {
            pcounter++;
        }
        else if (sleepPeriod != 10)
        {
            sleepPeriod = 10;
        }
        sleep(sleepPeriod);
    }
}

int main(int argc, char **argv)
{
    std::string dotenvPayload = readenv();
    nlohmann::json env = nlohmann::json::parse(dotenvPayload);

    Telegram telegram(env["bot"]["token"].get<std::string>());

    if (telegram.apiGetMe())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Telegram Bot Id: %lli\n", telegram.getId());
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Telegram Bot Name: %s\n", telegram.getName().c_str());
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "Telegram Bot Username: %s\n", telegram.getUsername().c_str());
        if (env["bot"].contains("webhook"))
        {
            if (env["bot"]["webhook"].contains("url"))
            {
                std::string url = env["bot"]["webhook"]["url"].get<std::string>();
                if (url.empty() == false)
                {
                    webhookRoutine(telegram, env["bot"]["webhook"]["url"].get<std::string>());
                }
            }
        }

        telegram.apiUnsetWebhook();
        getUpdatesRoutine(telegram);
    }
    else
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "Failed to access telegram!\n");
    }
    return 0;
}
