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

Request::Request(const std::string &url, const std::string &token, Request::REQUEST_t req)
{
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[req];
    FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
    api.setHiddenConfidential(token);
    this->success = api.get();
    if (this->success)
    {
        this->response = api.getPayload();
    }
}

Request::Request(const std::string &url, const std::string &token, Request::REQUEST_t req, const std::string &data)
{
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[req];
    FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
    api.setHiddenConfidential(token);
    api.insertHeader("Content-Type", "application/json");
    api.setBody(data);
    this->success = api.post();
    if (this->success)
    {
        this->response = api.getPayload();
    }
}

Request::Request(const std::string &url, const std::string &token, Request::REQUEST_t req, const nlohmann::json &data)
{
    this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[req];
    FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
    api.setHiddenConfidential(token);
    api.setBody(data.dump());
    this->success = api.sendFile();
    if (this->success)
    {
        this->response = api.getPayload();
    }
}

Request::Request(const std::string &url, const std::string &token, Request::REQUEST_t req, const std::string &ref, std::vector<unsigned char> &data)
{
    std::string mediaPath = ref;
    this->success = false;
    if (req == Request::DOWNLOAD_MEDIA_BY_FILE_ID)
    {
        this->url = url + (url.at(url.length() - 1) == '/' ? "bot" : "/bot") + token + "/" + reqStr[Request::GET_MEDIA_PATH];
        FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
        api.setHiddenConfidential(token);
        nlohmann::json data;
        data["file_id"] = ref;
        api.setBody(data.dump());
        this->success = api.post();
        if (this->success)
        {
            try
            {
                nlohmann::json json = nlohmann::json::parse(api.getPayload());
                JSONValidator jvalidator(__FILE__, __LINE__, __func__);

                nlohmann::json jsonResult = jvalidator.getObject(json, "result");
                mediaPath = jvalidator.get<std::string>(jsonResult, "file_path", "result");

                this->response = mediaPath;
                req = Request::DOWNLOAD_MEDIA_BY_PATH;
            }
            catch (const std::exception &e)
            {
                Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "failed to parse payload: %s!\n", e.what());
                return;
            }
        }
        else
        {
            return;
        }
    }
    if (req == Request::DOWNLOAD_MEDIA_BY_PATH)
    {
        this->url = url + (url.at(url.length() - 1) == '/' ? "file/bot" : "/file/bot") + token + "/" + mediaPath;
        FetchAPI api(this->url, CONNECTION_TIMEOUT, ALL_TIMEOUT);
        api.setHiddenConfidential(token);
        this->success = api.download(data);
        if (this->success)
        {
            this->response = mediaPath;
        }
    }
    return;
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