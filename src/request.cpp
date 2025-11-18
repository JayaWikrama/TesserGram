#include "fetch-api.hpp"
#include "request.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"
#include "utils/include/debug.hpp"
#include "utils/include/error.hpp"

#define CONNECTION_TIMEOUT 10
#define ALL_TIMEOUT 15

static const char *reqStr[] = {
    "getMe",
    "getUpdates",
    "sendMessage",
    "editMessageText",
    "sendChatAction",
    "getFile",
    "",
    "",
    "sendPhoto",
    "sendAudio",
    "sendVideo",
    "sendAnimation",
    "sendVoice",
    "sendDocument",
    "setWebhook",
    "deleteWebhook"};

Request::Request(const std::string &url, const std::string &token, Request::Type req)
{
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[static_cast<std::size_t>(req)];
    FetchAPI fapi(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);

    fapi.confidential(token)
        .get()
        .onSuccess(
            [this](const std::string &payload)
            { 
                this->success = true;
                this->response = payload; })
        .onError(
            [this](FetchAPI::ReturnCode code, const std::string &err)
            {
                this->success = false;
                Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "[%02X] %s\n", code, err.c_str()); });
}

Request::Request(const std::string &url, const std::string &token, Request::Type req, const std::string &data)
{
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[static_cast<std::size_t>(req)];
    FetchAPI fapi(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);

    fapi.confidential(token)
        .header("Content-Type", "application/json")
        .post(data)
        .onSuccess(
            [this](const std::string &payload)
            { 
                this->success = true;
                this->response = payload; })
        .onError(
            [this](FetchAPI::ReturnCode code, const std::string &err)
            {
                this->success = false;
                Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "[%02X] %s\n", code, err.c_str()); });
}

Request::Request(const std::string &url, const std::string &token, Request::Type req, const nlohmann::json &data)
{
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[static_cast<std::size_t>(req)];
    FetchAPI fapi(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);

    fapi.confidential(token)
        .upload(data.dump())
        .onSuccess(
            [this](const std::string &payload)
            { 
                this->success = true;
                this->response = payload; })
        .onError(
            [this](FetchAPI::ReturnCode code, const std::string &err)
            {
                this->success = false;
                Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "[%02X] %s\n", code, err.c_str()); });
}

Request::Request(const std::string &url, const std::string &token, Request::Type req, const std::string &ref, std::vector<unsigned char> &data)
{
    std::string mediaPath = ref;
    this->success = false;
    if (req == Request::Type::DOWNLOAD_MEDIA_BY_FILE_ID)
    {
        this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[static_cast<std::size_t>(Request::Type::GET_MEDIA_PATH)];
        FetchAPI fapi(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);

        nlohmann::json data;
        data["file_id"] = ref;

        fapi.confidential(token)
            .post(data.dump())
            .onSuccess(
                [&](const std::string &payload)
                { 
                    this->success = true;
                    try
                    {
                        nlohmann::json json = nlohmann::json::parse(payload);
                        JSONValidator jvalidator(__FILE__, __LINE__, __func__);

                        nlohmann::json jsonResult = jvalidator.getObject(json, "result");
                        mediaPath = jvalidator.get<std::string>(jsonResult, "file_path", "result");

                        this->response = mediaPath;
                        req = Request::Type::DOWNLOAD_MEDIA_BY_PATH;
                    }
                    catch (const std::exception &e)
                    {
                        this->success = false;
                        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "failed to parse payload: %s!\n", e.what());
                    } })
            .onError(
                [this](FetchAPI::ReturnCode code, const std::string &err)
                {
                    this->success = false;
                    Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "[%02X] %s\n", code, err.c_str()); });
    }
    if (req == Request::Type::DOWNLOAD_MEDIA_BY_PATH)
    {
        this->url = url + (url.at(url.length() - 1) == '/' ? "file/bot" : "/file/bot") + token + "/" + mediaPath;
        FetchAPI fapi(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);

        fapi.confidential(token)
            .download(data)
            .onSuccess(
                [&](const std::string &payload)
                { 
                    this->success = true;
                    this->response = mediaPath; })
            .onError(
                [this](FetchAPI::ReturnCode code, const std::string &err)
                {
                    this->success = false;
                    Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "[%02X] %s\n", code, err.c_str()); });
    }
}

Request::~Request()
{
}

bool Request::isSuccess()
{
    return this->success;
}

const std::string &Request::getResponse() const
{
    return this->response;
}