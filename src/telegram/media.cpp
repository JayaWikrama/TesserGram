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

bool Telegram::__apiSendMedia(long long targetId, Telegram::MediaType_t type, const std::string& label, const std::string& filePath){
    std::string rtype = "document";
    Request::REQUEST_t raction = Request::SEND_DOCUMENT;
    if (type == Telegram::PHOTO) {
        rtype = "photo";
        raction = Request::SEND_PHOTO;
    }
    else if (type == Telegram::AUDIO) {
        rtype = "audio";
        raction = Request::SEND_AUDIO;
    }
    else if (type == Telegram::VOICE) {
        rtype = "voice";
        raction = Request::SEND_VOICE;
    }
    else if (type == Telegram::ANIMATION) {
        rtype = "animation";
        raction = Request::SEND_ANIMATION;
    }
    else if (type == Telegram::VIDEO) {
        rtype = "video";
        raction = Request::SEND_VIDEO;
    }
    JsonBuilder mimeArray = {
        {{"name", "chat_id"}, {"is_file", false}, {"data", std::to_string(targetId)}},
        {{"name", "caption"}, {"is_file", false}, {"data", label}},
        {{"name", rtype}, {"is_file", true}, {"data", filePath}, {"type", getMimeType(filePath)}}
    };
    JsonBuilder data;
    data["mime"] = mimeArray;
    Request req(TELEGRAM_BASE_URL, this->token, raction, data);
    if (req.isSuccess()){
        std::cout << "success" << std::endl;
        return true;
    }
    return false;
}

bool Telegram::apiSendDocument(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Telegram::DOCUMENT, label, filePath);
}

bool Telegram::apiSendPhoto(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Telegram::PHOTO, label, filePath);
}
bool Telegram::apiSendAudio(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Telegram::AUDIO, label, filePath);
}
bool Telegram::apiSendVoice(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Telegram::VOICE, label, filePath);
}
bool Telegram::apiSendAnimation(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Telegram::ANIMATION, label, filePath);
}
bool Telegram::apiSendVideo(long long targetId, const std::string& label, const std::string& filePath){
    return this->__apiSendMedia(targetId, Telegram::VIDEO, label, filePath);
}