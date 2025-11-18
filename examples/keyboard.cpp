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

        TKeyboard keyboard(TKeyboard::Type::KEYBOARD, "Test Keyboard!");
        std::vector<std::string> buttons;
        buttons.push_back("Key-1");
        buttons.push_back("Key-2");

        keyboard
            .add(buttons)
            .add("Key-3")
            .add("Key-4");

        telegram.apiSendKeyboard(env["bot"]["target_id"].get<long long>(), keyboard);
    }
    else
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "Failed to access telegram!\n");
    }
    return 0;
}
