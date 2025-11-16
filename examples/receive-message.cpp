#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <thread>
#include "telegram.hpp"
#include "nlohmann/json.hpp"
#include "utils/include/debug.hpp"

int main(int argc, char **argv)
{
    nlohmann::json env = nlohmann::json::parse(std::ifstream(".env"));

    Telegram telegram(env["bot"]["token"].get<std::string>());

    if (telegram.apiGetMe())
    {
        telegram.info();
        telegram.clearUpdates();

        for (;;)
        {
            telegram.getUpdates(
                [](Telegram &t, const NodeMessage &message)
                {
                    message.display();
                    message
                        .processMessage(
                            [&](const Message &m)
                            {
                                t.apiSendMessage(m.chat.id, "Hi...");
                            })
                        .processCallbackQuery(
                            [&](const CallbackQuery &c)
                            {
                                t.apiSendMessage(c.message->chat.id, "Hello...");
                            });
                    exit(0);
                });
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    else
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "Failed to access telegram!\n");
    }
    return 0;
}
