#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include "telegram.hpp"
#include "debug.hpp"
#include "json-parser.hpp"

Debug debug(10);

std::string readenv() {
    std::ifstream file(".env");
    if (!file.is_open()) {
        debug.log(Debug::ERROR, __PRETTY_FUNCTION__, "ENV not found!\n");
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    return ss.str();
}

int main(int argc, char **argv){
    std::string dotenvPayload = readenv();
    JsonObject env(dotenvPayload);
    Telegram telegram(env["bot->token"].getString());
    telegram.enableDebug();
    if (telegram.apiGetMe()){
        debug.log(Debug::INFO, __PRETTY_FUNCTION__, "Telegram Bot Id: %lli\n", telegram.getId());
        debug.log(Debug::INFO, __PRETTY_FUNCTION__, "Telegram Bot Name: %s\n", telegram.getName().c_str());
        debug.log(Debug::INFO, __PRETTY_FUNCTION__, "Telegram Bot Username: %s\n", telegram.getUsername().c_str());
        
        telegram.apiSendMessage(std::stoll(env["bot->target_id"].getString()), "example message!");
    }
    else {
        debug.log(Debug::ERROR, __PRETTY_FUNCTION__, "Failed to access telegram!\n");
    }
    return 0;
}
