#include <stdexcept>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <unordered_map>
#include <cctype>
#include "telegram.hpp"
#include "request.hpp"
#include "json-parser.hpp"

static std::string getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    std::string ext = filename.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

static std::string getMimeType(const std::string& filename) {
    static const std::unordered_map<std::string, std::string> mimeMap = {
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif", "image/gif"},
        {"bmp", "image/bmp"},
        {"webp", "image/webp"},
        {"tiff", "image/tiff"},
        {"tif", "image/tiff"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},
        {"heic", "image/heic"},
        {"heif", "image/heif"},
        {"psd", "image/vnd.adobe.photoshop"},
        {"txt", "text/plain"},
        {"json", "application/json"},
        {"pdf", "application/pdf"},
        {"zip", "application/zip"}
    };

    std::string ext = getFileExtension(filename);
    auto it = mimeMap.find(ext);
    if (it != mimeMap.end()) {
        return it->second;
    }
    return "text/plain";
}

bool Telegram::__apiSendMedia(long long targetId, Request::REQUEST_t type, const std::string& label, const std::string& filePath){
    std::string rtype = "document";
    if (type == Request::SEND_PHOTO) rtype = "photo";
    else if (type == Request::SEND_AUDIO) rtype = "audio";
    else if (type == Request::SEND_VOICE) rtype = "voice";
    else if (type == Request::SEND_ANIMATION) rtype = "animation";
    else if (type == Request::SEND_VIDEO) rtype = "video";
    JsonBuilder mimeArray = {
        {{"name", "chat_id"}, {"is_file", false}, {"data", std::to_string(targetId)}},
        {{"name", "caption"}, {"is_file", false}, {"data", label}},
        {{"name", rtype}, {"is_file", true}, {"data", filePath}, {"type", getMimeType(filePath)}}
    };
    JsonBuilder data;
    data["mime"] = mimeArray;
    Request req(TELEGRAM_BASE_URL, this->token, type, data);
    if (req.isSuccess()){
        std::cout << "success" << std::endl;
        return true;
    }
    return false;
}

bool Telegram::apiSendDocument(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Request::SEND_DOCUMENT, label, filePath);
}

bool Telegram::apiSendPhoto(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Request::SEND_PHOTO, label, filePath);
}
bool Telegram::apiSendAudio(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Request::SEND_AUDIO, label, filePath);
}
bool Telegram::apiSendVoice(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Request::SEND_VOICE, label, filePath);
}
bool Telegram::apiSendAnimation(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Request::SEND_ANIMATION, label, filePath);
}
bool Telegram::apiSendVideo(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Request::SEND_VIDEO, label, filePath);
}