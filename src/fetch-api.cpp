#include <iostream>
#include <sstream>
#include <stdexcept>
#include <curl/curl.h>
#include "fetch-api.hpp"
#include "json-validator.hpp"
#include "nlohmann/json.hpp"
#include "utils/include/error.hpp"

FetchAPI::Mime::Mime() : file(false), name(), data(), type()
{
}

FetchAPI::Mime::~Mime() {}

bool FetchAPI::Mime::parse(const nlohmann::json &json)
{
  try
  {
    JSONValidator jvalidator(__FILE__, __LINE__, __func__);

    this->name = jvalidator.get<std::string>(json, "name");
    this->data = jvalidator.get<std::string>(json, "data");
    if (json.contains("content_type"))
      this->type = jvalidator.get<std::string>(json, "content_type");
    else
      this->type.clear();
    this->file = jvalidator.get<bool>(json, "is_file");
    return true;
  }
  catch (const std::exception &e)
  {
    Debug::log(Debug::WARNING, __FILE__, __LINE__, __func__, "parse error: %s!\n", e.what());
  }
  return false;
}

bool FetchAPI::Mime::isFile() const
{
  return this->file;
}

bool FetchAPI::Mime::isContainsType() const
{
  return !(this->type.empty());
}

const std::string &FetchAPI::Mime::getName() const
{
  return this->name;
}

const std::string &FetchAPI::Mime::getData() const
{
  return this->data;
}

const std::string &FetchAPI::Mime::getType() const
{
  return this->type;
}

FetchAPI::FetchAPI(const std::string &url, unsigned short connectTimeout, unsigned short totalTimeout) : debug(100), mutex()
{
  this->url = url;
  this->connectTimeout = connectTimeout;
  this->totalTimeout = totalTimeout;
  this->payload = "";
  this->errorMsg = "";
  this->errorCode = FetchAPI::FETCH_OK;
  this->headers.clear();
  this->body = "";
}

FetchAPI::~FetchAPI()
{
}

void FetchAPI::setHiddenConfidential(const std::string &confidential)
{
  std::lock_guard<std::mutex> guard(this->mutex);
  this->debug.setConfidential(confidential);
}

void FetchAPI::setHiddenConfidential(const std::vector<std::string> &confidential)
{
  std::lock_guard<std::mutex> guard(this->mutex);
  for (const std::string &conf : confidential)
    this->debug.setConfidential(conf);
}

void FetchAPI::insertHeader(const std::string &key, const std::string &value)
{
  std::lock_guard<std::mutex> guard(this->mutex);
  auto it = this->headers.find(key);
  if (it != this->headers.end())
  {
    it->second = value;
  }
  else
  {
    this->headers.insert({key, value});
  }
}

void FetchAPI::setBody(const std::string &data)
{
  std::lock_guard<std::mutex> guard(this->mutex);
  this->body = data;
}

size_t FetchAPI::writeCallback(void *contents, size_t size, size_t nmemb, std::string *s)
{
  size_t totalSize = size * nmemb;
  s->append((char *)contents, totalSize);
  return totalSize;
}

size_t FetchAPI::downloadCallback(void *contents, size_t size, size_t nmemb, std::vector<unsigned char> *v)
{
  size_t totalSize = size * nmemb;
  v->insert(v->end(), (unsigned char *)contents, (unsigned char *)contents + totalSize);
  return totalSize;
}

void FetchAPI::reset()
{
  this->payload.clear();
  this->errorMsg.clear();
  this->errorCode = FetchAPI::FETCH_OK;
}

bool FetchAPI::get(const std::map<std::string, std::string> &queryParams)
{
  std::lock_guard<std::mutex> guard(this->mutex);
  this->reset();
  CURL *curl = curl_easy_init();
  if (!curl)
  {
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "Failed to initialize CURL";
    this->debug.log(Debug::ERROR, __func__, "%s\n", this->errorMsg.c_str());
    return false;
  }

  std::ostringstream fullUrl;
  fullUrl << this->url;
  if (!(queryParams.empty()))
  {
    fullUrl << "?";
    for (auto it = queryParams.begin(); it != queryParams.end(); ++it)
    {
      if (it != queryParams.begin())
        fullUrl << "&";
      fullUrl << curl_easy_escape(curl, it->first.c_str(), 0)
              << "="
              << curl_easy_escape(curl, it->second.c_str(), 0);
    }
  }

  struct curl_slist *chunk = nullptr;
  for (const auto &pair : this->headers)
  {
    const auto &key = pair.first;
    const auto &val = pair.second;
    chunk = curl_slist_append(chunk, (key + ": " + val).c_str());
  }

  std::string response;
  char postField[2];
  postField[0] = 0x00;
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_URL, fullUrl.str().c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postField);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, this->connectTimeout);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, this->totalTimeout);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent-telegram/1.0");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FetchAPI::writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  if (chunk)
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  this->debug.log(Debug::INFO, __func__, "GET request to %s\n", fullUrl.str().c_str());

  CURLcode res = curl_easy_perform(curl);
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  if (res != CURLE_OK)
  {
    this->errorMsg = curl_easy_strerror(res);
    switch (res)
    {
    case CURLE_COULDNT_CONNECT:
      this->errorCode = FetchAPI::FETCH_ERR_CONNECTION;
      break;
    case CURLE_OPERATION_TIMEDOUT:
      this->errorCode = FetchAPI::FETCH_ERR_TIMEOUT;
      break;
    case CURLE_URL_MALFORMAT:
      this->errorCode = FetchAPI::FETCH_ERR_INVALID_URL;
      break;
    default:
      this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
      break;
    }
  }
  else if (httpCode >= 400)
  {
    this->errorCode = FetchAPI::FETCH_ERR_HTTP_ERROR;
    this->errorMsg = "HTTP error code: " + std::to_string(httpCode);
  }
  else
  {
    this->payload = response;
    this->errorCode = FetchAPI::FETCH_OK;
  }

  if (chunk)
    curl_slist_free_all(chunk);
  curl_easy_cleanup(curl);

  {
    if (this->errorCode == FetchAPI::FETCH_OK)
    {
      this->debug.log(Debug::INFO, __func__, "GET request to %s success\n", fullUrl.str().c_str());
      this->debug.log(Debug::INFO, __func__, "Response: %s\n", response.length() > 0 ? ("\n" + response).c_str() : "none");
    }
    else
    {
      this->debug.log(Debug::ERROR, __func__, "GET request to %s failed [%s]\n", fullUrl.str().c_str(), this->errorMsg.c_str());
      this->debug.log(Debug::ERROR, __func__, "Error body response: %s\n", response.length() > 0 ? ("\n" + response).c_str() : "none");
    }
  }

  return (this->errorCode == FetchAPI::FETCH_OK);
}

bool FetchAPI::post()
{
  std::lock_guard<std::mutex> guard(this->mutex);
  reset();
  CURL *curl = curl_easy_init();
  if (!curl)
  {
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "Failed to initialize CURL";
    return false;
  }

  struct curl_slist *chunk = nullptr;
  for (const auto &pair : this->headers)
  {
    const auto &key = pair.first;
    const auto &val = pair.second;
    chunk = curl_slist_append(chunk, (key + ": " + val).c_str());
  }

  std::string response;
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, this->body.c_str());
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, this->connectTimeout);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, this->totalTimeout);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent-telegram/1.0");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FetchAPI::writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  if (chunk)
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  this->debug.log(Debug::INFO, __func__, "POST request to %s with data %s\n", this->url.c_str(), this->body.c_str());

  CURLcode res = curl_easy_perform(curl);
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  if (res != CURLE_OK)
  {
    this->errorMsg = curl_easy_strerror(res);
    switch (res)
    {
    case CURLE_COULDNT_CONNECT:
      this->errorCode = FetchAPI::FETCH_ERR_CONNECTION;
      break;
    case CURLE_OPERATION_TIMEDOUT:
      this->errorCode = FetchAPI::FETCH_ERR_TIMEOUT;
      break;
    case CURLE_URL_MALFORMAT:
      this->errorCode = FetchAPI::FETCH_ERR_INVALID_URL;
      break;
    default:
      this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
      break;
    }
  }
  else if (httpCode >= 400)
  {
    this->errorCode = FetchAPI::FETCH_ERR_HTTP_ERROR;
    this->errorMsg = "HTTP error code: " + std::to_string(httpCode);
  }
  else
  {
    this->payload = response;
    this->errorCode = FetchAPI::FETCH_OK;
  }

  {
    if (this->errorCode == FetchAPI::FETCH_OK)
    {
      this->debug.log(Debug::INFO, __func__, "POST request to %s success\n", this->url.c_str());
      this->debug.log(Debug::INFO, __func__, "Response: %s\n", response.length() > 0 ? ("\n" + response).c_str() : "none");
    }
    else
    {
      this->debug.log(Debug::ERROR, __func__, "POST request to %s failed [%s]\n", this->url.c_str(), this->errorMsg.c_str());
      this->debug.log(Debug::ERROR, __func__, "Error body response: %s\n", response.length() > 0 ? ("\n" + response).c_str() : "none");
    }
  }

  if (chunk)
    curl_slist_free_all(chunk);
  curl_easy_cleanup(curl);

  return (this->errorCode == FetchAPI::FETCH_OK);
}

bool FetchAPI::sendFile()
{
  std::lock_guard<std::mutex> guard(this->mutex);
  reset();
  if (body.length() == 0)
  {
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "body empty";
    return false;
  }

  nlohmann::json json;
  nlohmann::json jmime;
  try
  {
    json = nlohmann::json::parse(this->body);

    JSONValidator jvalidator(__FILE__, __LINE__, __func__);
    jmime = jvalidator.getArray(json, "mime");
  }
  catch (const std::exception &e)
  {
    this->debug.log(Debug::ERROR, __func__, "mime parse failed: %s!\n", e.what());
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "invalid json format";
    return false;
  }
  curl_mime *mime = nullptr;
  curl_mimepart *part = nullptr;
  CURL *curl = curl_easy_init();
  if (!curl)
  {
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "Failed to initialize CURL";
    return false;
  }

  struct curl_slist *chunk = nullptr;
  for (const auto &pair : this->headers)
  {
    const auto &key = pair.first;
    const auto &val = pair.second;
    if (key != std::string("Content-Type") && key != std::string("Content-type") && key != std::string("content-type"))
    {
      chunk = curl_slist_append(chunk, (key + ": " + val).c_str());
    }
  }

  std::string response;
  curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, this->connectTimeout);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, this->totalTimeout);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent-telegram/1.0");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FetchAPI::writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  if (chunk)
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  mime = curl_mime_init(curl);

  FetchAPI::Mime mimeObject;

  for (const auto &j : jmime)
  {
    if (mimeObject.parse(j))
    {
      part = curl_mime_addpart(mime);
      curl_mime_name(part, mimeObject.getName().c_str());
      if (mimeObject.isFile())
        curl_mime_filedata(part, mimeObject.getData().c_str());
      else
        curl_mime_data(part, mimeObject.getData().c_str(), CURL_ZERO_TERMINATED);
      if (mimeObject.isContainsType())
        curl_mime_type(part, mimeObject.getType().c_str());
    }
  }

  curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

  this->debug.log(Debug::INFO, __func__, "POST request to %s with data %s\n", this->url.c_str(), this->body.c_str());

  CURLcode res = curl_easy_perform(curl);
  curl_mime_free(mime);
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  if (res != CURLE_OK)
  {
    this->errorMsg = curl_easy_strerror(res);
    switch (res)
    {
    case CURLE_COULDNT_CONNECT:
      this->errorCode = FetchAPI::FETCH_ERR_CONNECTION;
      break;
    case CURLE_OPERATION_TIMEDOUT:
      this->errorCode = FetchAPI::FETCH_ERR_TIMEOUT;
      break;
    case CURLE_URL_MALFORMAT:
      this->errorCode = FetchAPI::FETCH_ERR_INVALID_URL;
      break;
    default:
      this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
      break;
    }
  }
  else if (httpCode >= 400)
  {
    this->errorCode = FetchAPI::FETCH_ERR_HTTP_ERROR;
    this->errorMsg = "HTTP error code: " + std::to_string(httpCode);
  }
  else
  {
    this->payload = response;
    this->errorCode = FetchAPI::FETCH_OK;
  }

  {
    if (this->errorCode == FetchAPI::FETCH_OK)
    {
      this->debug.log(Debug::INFO, __func__, "POST request to %s success\n", this->url.c_str());
    }
    else
    {
      this->debug.log(Debug::ERROR, __func__, "POST request to %s failed [%s]\n", this->url.c_str(), this->errorMsg.c_str());
      this->debug.log(Debug::ERROR, __func__, "Error body response: %s\n", response.length() > 0 ? ("\n" + response).c_str() : "none");
    }
  }

  if (chunk)
    curl_slist_free_all(chunk);
  curl_easy_cleanup(curl);

  return (this->errorCode == FetchAPI::FETCH_OK);
}

bool FetchAPI::download(std::vector<unsigned char> &data, const std::map<std::string, std::string> &params)
{
  std::lock_guard<std::mutex> guard(this->mutex);
  this->reset();
  data.clear();
  CURL *curl = curl_easy_init();
  if (!curl)
  {
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "Failed to initialize CURL";
    this->debug.log(Debug::ERROR, __func__, "%s\n", this->errorMsg.c_str());
    return false;
  }

  std::ostringstream fullUrl;
  fullUrl << this->url;
  if (!(params.empty()))
  {
    fullUrl << "?";
    for (auto it = params.begin(); it != params.end(); ++it)
    {
      if (it != params.begin())
        fullUrl << "&";
      fullUrl << curl_easy_escape(curl, it->first.c_str(), 0)
              << "="
              << curl_easy_escape(curl, it->second.c_str(), 0);
    }
  }

  struct curl_slist *chunk = nullptr;
  for (const auto &pair : this->headers)
  {
    const auto &key = pair.first;
    const auto &val = pair.second;
    chunk = curl_slist_append(chunk, (key + ": " + val).c_str());
  }

  char postField[2];
  postField[0] = 0x00;
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_URL, fullUrl.str().c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postField);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, this->connectTimeout);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, this->totalTimeout);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent-telegram/1.0");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FetchAPI::downloadCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
  if (chunk)
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  this->debug.log(Debug::INFO, __func__, "GET request to %s\n", fullUrl.str().c_str());

  CURLcode res = curl_easy_perform(curl);
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  if (res != CURLE_OK)
  {
    this->errorMsg = curl_easy_strerror(res);
    switch (res)
    {
    case CURLE_COULDNT_CONNECT:
      this->errorCode = FetchAPI::FETCH_ERR_CONNECTION;
      break;
    case CURLE_OPERATION_TIMEDOUT:
      this->errorCode = FetchAPI::FETCH_ERR_TIMEOUT;
      break;
    case CURLE_URL_MALFORMAT:
      this->errorCode = FetchAPI::FETCH_ERR_INVALID_URL;
      break;
    default:
      this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
      break;
    }
  }
  else if (httpCode >= 400)
  {
    this->errorCode = FetchAPI::FETCH_ERR_HTTP_ERROR;
    this->errorMsg = "HTTP error code: " + std::to_string(httpCode);
  }
  else
  {
    this->payload = "";
    this->errorCode = FetchAPI::FETCH_OK;
  }

  if (chunk)
    curl_slist_free_all(chunk);
  curl_easy_cleanup(curl);

  {
    if (this->errorCode == FetchAPI::FETCH_OK)
    {
      this->debug.log(Debug::INFO, __func__, "GET request (download) to %s success\n", fullUrl.str().c_str());
      this->debug.log(Debug::INFO, __func__, "Download size: %lu\n", data.size());
    }
    else
    {
      this->debug.log(Debug::ERROR, __func__, "GET request (download) to %s failed [%s]\n", fullUrl.str().c_str(), this->errorMsg.c_str());
    }
  }

  return (this->errorCode == FetchAPI::FETCH_OK);
}

std::string FetchAPI::getPayload()
{
  std::string result;
  std::lock_guard<std::mutex> guard(this->mutex);
  result = this->payload;
  return result;
}

std::string FetchAPI::getError()
{
  std::string result;
  std::lock_guard<std::mutex> guard(this->mutex);
  result = this->errorMsg;
  return result;
}

FetchAPI::FetchErrorCode_t FetchAPI::getErrorCode()
{
  FetchAPI::FetchErrorCode_t result;
  std::lock_guard<std::mutex> guard(this->mutex);
  result = this->errorCode;
  return result;
}