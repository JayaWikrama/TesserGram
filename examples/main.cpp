#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "json-parser.hpp"
#include "telegram.hpp"

std::string readenv() {
    std::ifstream file(".env");
    if (!file.is_open()) {
        std::cerr << "ENV not found!" << std::endl;
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    return ss.str();
}


int main(int argc, char **argv){
    char *buffer = NULL;
    std::string dotenvPayload = readenv();
    std::cout << "ENV Payload:" << std::endl;
    std::cout << dotenvPayload << std::endl;
    JsonObject env(dotenvPayload);
    Telegram telegram(env["bot->token"].getString());
    if (telegram.apiGetMe()){
        std::cout << "Telegram Bot Id: " << telegram.getId() << std::endl;
        std::cout << "Telegram Bot Name: " << telegram.getName() << std::endl;
        std::cout << "Telegram Bot Username: " << telegram.getUsername() << std::endl;
        telegram.apigetUpdates();
        const NodeMessage *msg = telegram.message.getMessage();
        while (msg){
            msg->display();
            telegram.message.dequeue();
            msg = telegram.message.getMessage();
        }
        telegram.apiSendMessage(1030198712, "test");
    }
    else {
        std::cout << "Failed to access telegram" << std::endl;
    }
    return 0;
}
