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

        TKeyboard keyboard(TKeyboard::INLINE_KEYBOARD, "Test Keyboard!");
        keyboard.addButton(TKeyboard::URL, "Google", "www.google.com");

        TKeyboard::TKeyButtonConstructor_t button;
        std::vector<TKeyboard::TKeyButtonConstructor_t> buttons;
        button.type = TKeyboard::CALLBACK_QUERY;
        button.text = "No!";
        button.value = "no";
        buttons.push_back(button);
        button.type = TKeyboard::CALLBACK_QUERY;
        button.text = "Yes!";
        button.value = "yes";
        buttons.push_back(button);
        keyboard.addButton(buttons);

        telegram.apiSendKeyboard(env["bot"]["target_id"].get<long long>(), keyboard);
    }
    else
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "Failed to access telegram!\n");
    }
    return 0;
}
