#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
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

        telegram.apiSendDocument(env["bot"]["target_id"].get<long long>(), "Read this document!", "../README.md");
        telegram.apiSendPhoto(env["bot"]["target_id"].get<long long>(), "This the picture!", "../docs/images/typing_chat_action.jpeg");
    }
    else
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "Failed to access telegram!\n");
    }
    return 0;
}
