/*
 * $Id: debug.cpp,v 1.0.0 2024/12/26 20:55:23 Jaya Wikrama Exp $
 *
 * Copyright (c) 2024 Jaya Wikrama
 * jayawikrama89@gmail.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <iostream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include "debug.hpp"

Debug::Debug(){
    this->maxLineLogs = 1000;
    this->history.clear();
    pthread_mutex_init(&(this->mutex), NULL);
}

Debug::Debug(size_t maxLineLogs){
    this->maxLineLogs = maxLineLogs;
    this->history.clear();
    pthread_mutex_init(&(this->mutex), NULL);
}

Debug::~Debug(){
    pthread_mutex_lock(&(this->mutex));
    this->history.clear();
    pthread_mutex_unlock(&(this->mutex));
    pthread_mutex_destroy(&(this->mutex));
}

std::string Debug::logTypeToString(Debug::LogType_t type) const {
    switch (type){
        case Debug::INFO: return "I";
        case Debug::WARNING: return "W";
        case Debug::ERROR: return "E";
        case Debug::CRITICAL: return "C";
    }
    return "UNKNOWN";
}

void Debug::log(Debug::LogType_t type, const char* functionName, const char* format, ...){
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    char timeBuffer[20];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%y%m%d_%H%M%S", localTime);

    va_list args;
    va_start(args, format);
    int messageSize = std::vsnprintf(nullptr, 0, format, args) + 1;
    va_end(args);

    std::vector<char> messageBuffer(messageSize);
    va_start(args, format);
    std::vsnprintf(messageBuffer.data(), messageBuffer.size(), format, args);
    va_end(args);

    pthread_mutex_lock(&(this->mutex));
    std::ostringstream oss;
    static char *functionNameMod = nullptr;
    if (functionNameMod == nullptr){
        functionNameMod = (char *) malloc(strlen(functionName) + 1);
    }
    else {
        functionNameMod = (char *) realloc(functionNameMod, strlen(functionName) + 1);
    }
    if (functionNameMod){
        strcpy(functionNameMod, functionName);
        functionNameMod[strlen(functionName)] = 0x00;
        static char *tmp = nullptr;
        tmp = strstr(functionNameMod, " ");
        if (tmp == nullptr){
            tmp = functionNameMod;
        }
        else {
            tmp += 1;
        }
        static char *tmp2 = nullptr;
        tmp2 = strstr(tmp, "(");
        if (tmp2 != nullptr){
            tmp2[0] = 0x00;
        }
        oss << "[" << timeBuffer << "] [" << logTypeToString(type)
            << "]: " << tmp << ": " << messageBuffer.data();
    }
    else {
        oss << "[" << timeBuffer << "] [" << logTypeToString(type)
            << "]: " << functionName << ": " << messageBuffer.data();
    }
    std::string logEntry = oss.str();

    std::cout << logEntry;

    if (this->history.size() >= this->maxLineLogs){
        this->history.erase(this->history.begin());
    }
    this->history.push_back(logEntry);
    pthread_mutex_unlock(&(this->mutex));
}

std::string Debug::getLogHistory(){
    pthread_mutex_lock(&(this->mutex));
    std::ostringstream oss;
    for (const auto& log : this->history){
        oss << log;
    }
    pthread_mutex_unlock(&(this->mutex));
    return oss.str();
}

void Debug::clearLogHistory(){
    pthread_mutex_lock(&(this->mutex));
    this->history.clear();
    pthread_mutex_unlock(&(this->mutex));
}