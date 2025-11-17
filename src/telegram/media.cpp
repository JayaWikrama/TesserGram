#include <stdexcept>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <unordered_map>
#include <cctype>
#include "telegram.hpp"
#include "request.hpp"
#include "utils/include/debug.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"

static std::string getFileExtension(const std::string &filename)
{
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos)
        return "";
    std::string ext = filename.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

static std::string getMimeType(const std::string &filename)
{
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
        {"zip", "application/zip"}};

    std::string ext = getFileExtension(filename);
    auto it = mimeMap.find(ext);
    if (it != mimeMap.end())
    {
        return it->second;
    }
    return "text/plain";
}

bool Telegram::__apiSendMedia(long long targetId, Media::Type type, const std::string &label, const std::string &filePath)
{
    Request::Type raction = Request::Type::SEND_DOCUMENT;

    switch (type)
    {
    case Media::Type::PHOTO:
        raction = Request::Type::SEND_PHOTO;
        break;
    case Media::Type::AUDIO:
        raction = Request::Type::SEND_AUDIO;
        break;
    case Media::Type::VOICE:
        raction = Request::Type::SEND_VOICE;
        break;
    case Media::Type::ANIMATION:
        raction = Request::Type::SEND_ANIMATION;
        break;
    case Media::Type::VIDEO:
        raction = Request::Type::SEND_VIDEO;
    default:
        break;
    }

    nlohmann::json mimeArray = {
        {{"name", "chat_id"}, {"is_file", false}, {"data", std::to_string(targetId)}},
        {{"name", "caption"}, {"is_file", false}, {"data", label}},
        {{"name", Media::typeToString(type)}, {"is_file", true}, {"data", filePath}, {"type", getMimeType(filePath)}}};
    nlohmann::json data;
    data["mime"] = mimeArray;
    Request req(TELEGRAM_BASE_URL, this->token, raction, data);
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        return true;
    }
    return false;
}

bool Telegram::apiSendDocument(long long targetId, const std::string &label, const std::string &filePath)
{
    return this->__apiSendMedia(targetId, Media::Type::DOCUMENT, label, filePath);
}

bool Telegram::apiSendPhoto(long long targetId, const std::string &label, const std::string &filePath)
{
    return this->__apiSendMedia(targetId, Media::Type::PHOTO, label, filePath);
}
bool Telegram::apiSendAudio(long long targetId, const std::string &label, const std::string &filePath)
{
    return this->__apiSendMedia(targetId, Media::Type::AUDIO, label, filePath);
}
bool Telegram::apiSendVoice(long long targetId, const std::string &label, const std::string &filePath)
{
    return this->__apiSendMedia(targetId, Media::Type::VOICE, label, filePath);
}
bool Telegram::apiSendAnimation(long long targetId, const std::string &label, const std::string &filePath)
{
    return this->__apiSendMedia(targetId, Media::Type::ANIMATION, label, filePath);
}
bool Telegram::apiSendVideo(long long targetId, const std::string &label, const std::string &filePath)
{
    return this->__apiSendMedia(targetId, Media::Type::VIDEO, label, filePath);
}

std::string Telegram::apiGetMediaPath(const std::string &fileId)
{
    nlohmann::json data;
    data["file_id"] = fileId;
    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::GET_MEDIA_PATH, data.dump());
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
        try
        {
            nlohmann::json json = nlohmann::json::parse(req.getResponse());
            JSONValidator jvalidator(__FILE__, __LINE__, __func__);

            const nlohmann::json &jsonResult = jvalidator.getObject(json, "result");
            return jvalidator.get<std::string>(jsonResult, "file_path");
        }
        catch (const std::exception &e)
        {
            Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "parse failed: %s!\n", e.what());
        }
    }
    return "";
}

std::vector<unsigned char> Telegram::apiDownloadMediaById(const std::string &fileId)
{
    std::vector<unsigned char> result;
    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::DOWNLOAD_MEDIA_BY_FILE_ID, fileId, result);
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
    }
    return result;
}

std::vector<unsigned char> Telegram::apiDownloadMediaByPath(const std::string &mediaPath)
{
    std::vector<unsigned char> result;
    Request req(TELEGRAM_BASE_URL, this->token, Request::Type::DOWNLOAD_MEDIA_BY_PATH, mediaPath, result);
    if (req.isSuccess())
    {
        Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "success\n");
    }
    return result;
}