#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
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

        TKeyboard keyboard(TKeyboard::INLINE_KEYBOARD, "Test Keyboard!");

        std::vector<TKeyboard::TKeyButton> buttons;
        buttons.push_back(TKeyboard::TKeyButton(TKeyboard::TKeyButton::CALLBACK_QUERY, "No!", "no"));
        buttons.push_back(TKeyboard::TKeyButton(TKeyboard::TKeyButton::CALLBACK_QUERY, "Yes!", "yes"));

        keyboard
            .add(TKeyboard::TKeyButton::URL, "Google", "www.google.com")
            .add(buttons);

        telegram.apiSendKeyboard(env["bot"]["target_id"].get<long long>(), keyboard);
    }
    else
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "Failed to access telegram!\n");
    }
    return 0;
}
