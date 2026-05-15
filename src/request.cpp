#include "request.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"
#include "utils/include/debug.hpp"
#include "utils/include/error.hpp"
#include "fetchapi/include/fetch-api.hpp"

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
                this->response = payload;
                Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "response: %s\n", payload.c_str()); })
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

    Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "request: %s\n", data.c_str());

    fapi.confidential(token)
        .header("Content-Type", "application/json")
        .post(data)
        .onSuccess(
            [this](const std::string &payload)
            { 
                this->success = true;
                this->response = payload;
                Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "response: %s\n", payload.c_str()); })
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

    if (!data.is_array())
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "upload data must be a JSON array of MIME parts\n");
        return;
    }
    fapi.confidential(token)
        .upload(data.dump())
        .onSuccess(
            [this](const std::string &payload)
            { 
                this->success = true;
                this->response = payload;
                Debug::log(Debug::INFO, __FILE__, __LINE__, __func__, "response: %s\n", payload.c_str()); })
        .onError(
            [this](FetchAPI::ReturnCode code, const std::string &err)
            {
                this->success = false;
                Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "[%02X] %s\n", code, err.c_str()); });
}

static std::string fetchMediaPath(const std::string &url, const std::string &token, const std::string &fileId)
{
    std::string apiUrl = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[static_cast<std::size_t>(Request::Type::GET_MEDIA_PATH)];
    FetchAPI fapi(apiUrl, CONNECTION_TIMEOUT, ALL_TIMEOUT);

    nlohmann::json body;
    body["file_id"] = fileId;

    std::string mediaPath;
    fapi.confidential(token)
        .post(body.dump())
        .onSuccess(
            [&](const std::string &payload)
            {
                try
                {
                    nlohmann::json json = nlohmann::json::parse(payload);
                    JSONValidator jval(__FILE__, __LINE__, __func__);
                    nlohmann::json jsonResult = jval.getObject(json, "result");
                    mediaPath = jval.get<std::string>(jsonResult, "file_path", "result");
                }
                catch (const std::exception &e)
                {
                    Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "failed to parse payload: %s!\n", e.what());
                }
            })
        .onError(
            [](FetchAPI::ReturnCode code, const std::string &err)
            {
                Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "[%02X] %s\n", code, err.c_str());
            });

    return mediaPath;
}

Request::Request(const std::string &url, const std::string &token, Request::Type req, const std::string &ref, std::vector<unsigned char> &data)
{
    this->success = false;
    std::string mediaPath = ref;

    if (req == Request::Type::DOWNLOAD_MEDIA_BY_FILE_ID)
    {
        mediaPath = fetchMediaPath(url, token, ref);
        if (mediaPath.empty())
            return;
    }

    this->url = url + (url.at(url.length() - 1) == '/' ? "file/bot" : "/file/bot") + token + "/" + mediaPath;
    FetchAPI fapi(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);

    fapi.confidential(token)
        .download(data)
        .onSuccess(
            [&](const std::string &payload)
            {
                this->success = true;
                this->response = mediaPath;
            })
        .onError(
            [this](FetchAPI::ReturnCode code, const std::string &err)
            {
                this->success = false;
                Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "[%02X] %s\n", code, err.c_str());
            });
}

Request::~Request()
{
}

bool Request::isSuccess() const
{
    return this->success;
}

const std::string &Request::getResponse() const
{
    return this->response;
}