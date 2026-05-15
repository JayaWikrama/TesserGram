#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include "telegram.hpp"
#include "utils/include/nlohmann/json.hpp"
#include "utils/include/debug.hpp"

int main(int argc, char **argv)
{
    nlohmann::json env = nlohmann::json::parse(std::ifstream(".env"));

    Telegram telegram(env["bot"]["token"].get<std::string>());

    if (telegram.apiGetMe())
    {
        telegram.info();

        const nlohmann::json &jTarget = env["bot"]["target_id"];
        long long targetId = jTarget.is_number() ? jTarget.get<long long>() : std::stoll(jTarget.get<std::string>());
        telegram.apiSendDocument(targetId, "Read this document!", "../README.md");
        telegram.apiSendPhoto(targetId, "This the picture!", "../docs/images/typing_chat_action.jpeg");
    }
    else
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "Failed to access telegram!\n");
    }
    return 0;
}
