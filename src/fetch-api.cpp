#include <iostream>
#include <sstream>
#include <stdexcept>
#include <curl/curl.h>
#include "fetch-api.hpp"
#include "json-parser.hpp"

FetchAPI::FetchAPI(const std::string &url, unsigned short connectTimeout, unsigned short totalTimeout){
  if (pthread_mutex_init(&(this->mutex), nullptr)) throw std::runtime_error("Failed to initialize mutex!");
  pthread_mutex_lock(&(this->mutex));
  this->url = url;
  this->connectTimeout = connectTimeout;
  this->totalTimeout = totalTimeout;
  this->payload = "";
  this->errorMsg = "";
  this->errorCode = FetchAPI::FETCH_OK;
  this->headers.clear();
  this->body = "";
  this->debug = nullptr;
  pthread_mutex_unlock(&(this->mutex));
}

FetchAPI::~FetchAPI() {
  pthread_mutex_lock(&(this->mutex));
  pthread_mutex_unlock(&(this->mutex));
  pthread_mutex_destroy(&(this->mutex));
}

void FetchAPI::enableDebug(){
  pthread_mutex_lock(&(this->mutex));
  this->debug = new Debug(100);
  pthread_mutex_unlock(&(this->mutex));
}

void FetchAPI::disableDebug(){
  pthread_mutex_lock(&(this->mutex));
  delete (this->debug);
  this->debug = nullptr;
  pthread_mutex_unlock(&(this->mutex));
}

bool FetchAPI::isDebugEnable(){
  pthread_mutex_lock(&(this->mutex));
  bool result = (this->debug != nullptr);
  pthread_mutex_unlock(&(this->mutex));
  return result;
}

Debug& FetchAPI::DEBUG(){
  pthread_mutex_lock(&(this->mutex));
  if (this->debug == nullptr) throw std::runtime_error("Debug is disabled!");
  pthread_mutex_unlock(&(this->mutex));
  return *(this->debug);
}

void FetchAPI::insertHeader(const std::string &key, const std::string &value){
  pthread_mutex_lock(&(this->mutex));
  auto it = this->headers.find(key);
  if (it != this->headers.end()) {
    it->second = value;
  } else {
    this->headers.insert({key, value});
  }
  pthread_mutex_unlock(&(this->mutex));
}

void FetchAPI::setBody(const std::string &data){
  pthread_mutex_lock(&(this->mutex));
  this->body = data;
  pthread_mutex_unlock(&(this->mutex));
}

size_t FetchAPI::writeCallback(void *contents, size_t size, size_t nmemb, std::string *s){
  size_t totalSize = size * nmemb;
  s->append((char *)contents, totalSize);
  return totalSize;
}

void FetchAPI::reset(){
  this->payload.clear();
  this->errorMsg.clear();
  this->errorCode = FetchAPI::FETCH_OK;
}

bool FetchAPI::get(const std::map<std::string, std::string> &queryParams){
  pthread_mutex_lock(&(this->mutex));
  this->reset();
  CURL *curl = curl_easy_init();
  if (!curl) {
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "Failed to initialize CURL";
    if (this->debug) this->debug->log(Debug::ERROR, __PRETTY_FUNCTION__, "%s\n", this->errorMsg.c_str());
    pthread_mutex_unlock(&(this->mutex));
    return false;
  }

  std::ostringstream fullUrl;
  fullUrl << this->url;
  if (!(queryParams.empty())) {
    fullUrl << "?";
    for (auto it = queryParams.begin(); it != queryParams.end(); ++it) {
      if (it != queryParams.begin()) fullUrl << "&";
      fullUrl << curl_easy_escape(curl, it->first.c_str(), 0)
              << "="
              << curl_easy_escape(curl, it->second.c_str(), 0);
    }
  }

  struct curl_slist *chunk = nullptr;
  for (const auto &pair : this->headers) {
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
  if (chunk) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  if (this->debug) this->debug->log(Debug::INFO, __PRETTY_FUNCTION__, "GET request to %s\n", fullUrl.str().c_str());

  CURLcode res = curl_easy_perform(curl);
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  if (res != CURLE_OK) {
    this->errorMsg = curl_easy_strerror(res);
    switch (res) {
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
  } else if (httpCode >= 400) {
    this->errorCode = FetchAPI::FETCH_ERR_HTTP_ERROR;
    this->errorMsg = "HTTP error code: " + std::to_string(httpCode);
  } else {
    this->payload = response;
    this->errorCode = FetchAPI::FETCH_OK;
  }

  if (chunk) curl_slist_free_all(chunk);
  curl_easy_cleanup(curl);

  if (this->debug){
    if (this->errorCode == FetchAPI::FETCH_OK){
      this->debug->log(Debug::INFO, __PRETTY_FUNCTION__, "GET request to %s success\n", fullUrl.str().c_str());
    }
    else {
      this->debug->log(Debug::ERROR, __PRETTY_FUNCTION__, "GET request to %s failed [%s]\n", fullUrl.str().c_str(), this->errorMsg.c_str());
      this->debug->log(Debug::ERROR, __PRETTY_FUNCTION__, "Error body response: %s\n", response.length() > 0 ? ("\n" + response).c_str() : "none");
    }
  }

  pthread_mutex_unlock(&(this->mutex));
  return (this->errorCode == FetchAPI::FETCH_OK);
}

bool FetchAPI::post() {
  pthread_mutex_lock(&(this->mutex));
  reset();
  CURL *curl = curl_easy_init();
  if (!curl) {
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "Failed to initialize CURL";
    pthread_mutex_unlock(&(this->mutex));
    return false;
  }

  struct curl_slist *chunk = nullptr;
  for (const auto &pair : this->headers) {
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
  if (chunk) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  if (this->debug) this->debug->log(Debug::INFO, __PRETTY_FUNCTION__, "POST request to %s with data %s\n", this->url.c_str(), this->body.c_str());

  CURLcode res = curl_easy_perform(curl);
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  if (res != CURLE_OK) {
    this->errorMsg = curl_easy_strerror(res);
    switch (res) {
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
  } else if (httpCode >= 400) {
    this->errorCode = FetchAPI::FETCH_ERR_HTTP_ERROR;
    this->errorMsg = "HTTP error code: " + std::to_string(httpCode);
  } else {
    this->payload = response;
    this->errorCode = FetchAPI::FETCH_OK;
  }

  if (this->debug){
    if (this->errorCode == FetchAPI::FETCH_OK){
      this->debug->log(Debug::INFO, __PRETTY_FUNCTION__, "POST request to %s success\n", this->url.c_str());
    }
    else {
      this->debug->log(Debug::ERROR, __PRETTY_FUNCTION__, "POST request to %s failed [%s]\n", this->url.c_str(), this->errorMsg.c_str());
      this->debug->log(Debug::ERROR, __PRETTY_FUNCTION__, "Error body response: %s\n", response.length() > 0 ? ("\n" + response).c_str() : "none");
    }
  }

  if (chunk) curl_slist_free_all(chunk);
  curl_easy_cleanup(curl);

  pthread_mutex_unlock(&(this->mutex));
  return (this->errorCode == FetchAPI::FETCH_OK);
}

bool FetchAPI::sendFile() {
  pthread_mutex_lock(&(this->mutex));
  reset();
  if (body.length() == 0){
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "body empty";
    pthread_mutex_unlock(&(this->mutex));
    return false;
  }
  JsonObject json(this->body);
  int arrayLength = json["mime"].getArraySize();
  if (arrayLength == 0){
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "invalid json format";
    pthread_mutex_unlock(&(this->mutex));
    return false;
  }
  curl_mime *mime = nullptr;
  curl_mimepart *part = nullptr;
  CURL *curl = curl_easy_init();
  if (!curl) {
    this->errorCode = FetchAPI::FETCH_ERR_UNKNOWN;
    this->errorMsg = "Failed to initialize CURL";
    pthread_mutex_unlock(&(this->mutex));
    return false;
  }

  struct curl_slist *chunk = nullptr;
  for (const auto &pair : this->headers) {
    const auto &key = pair.first;
    const auto &val = pair.second;
    if (key != std::string("Content-Type") && key != std::string("Content-type") && key != std::string("content-type")){
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
  if (chunk) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  mime = curl_mime_init(curl);

  for (int i = 0; i < arrayLength; i++) {
    part = curl_mime_addpart(mime);
    curl_mime_name(part, json["mime[" + std::to_string(i) + "]->name"].getString().c_str());
    if (json["mime[" + std::to_string(i) + "]->is_file"].getString() == "true"){
      curl_mime_filedata(part, json["mime[" + std::to_string(i) + "]->data"].getString().c_str());
    }
    else {
      curl_mime_data(part, json["mime[" + std::to_string(i) + "]->data"].getString().c_str(), CURL_ZERO_TERMINATED);
    }
    if (json["mime[" + std::to_string(i) + "]->content_type"].isAvailable()){
      curl_mime_type(part, json["mime[" + std::to_string(i) + "]->content_type"].getString().c_str());
    }
  }

  curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

  if (this->debug) this->debug->log(Debug::INFO, __PRETTY_FUNCTION__, "POST request to %s with data %s\n", this->url.c_str(), this->body.c_str());

  CURLcode res = curl_easy_perform(curl);
  curl_mime_free(mime);
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  if (res != CURLE_OK) {
    this->errorMsg = curl_easy_strerror(res);
    switch (res) {
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
  } else if (httpCode >= 400) {
    this->errorCode = FetchAPI::FETCH_ERR_HTTP_ERROR;
    this->errorMsg = "HTTP error code: " + std::to_string(httpCode);
  } else {
    this->payload = response;
    this->errorCode = FetchAPI::FETCH_OK;
  }

  if (this->debug){
    if (this->errorCode == FetchAPI::FETCH_OK){
      this->debug->log(Debug::INFO, __PRETTY_FUNCTION__, "POST request to %s success\n", this->url.c_str());
    }
    else {
      this->debug->log(Debug::ERROR, __PRETTY_FUNCTION__, "POST request to %s failed [%s]\n", this->url.c_str(), this->errorMsg.c_str());
      this->debug->log(Debug::ERROR, __PRETTY_FUNCTION__, "Error body response: %s\n", response.length() > 0 ? ("\n" + response).c_str() : "none");
    }
  }

  if (chunk) curl_slist_free_all(chunk);
  curl_easy_cleanup(curl);

  pthread_mutex_unlock(&(this->mutex));
  return (this->errorCode == FetchAPI::FETCH_OK);
}

std::string FetchAPI::getPayload() {
  std::string result;
  pthread_mutex_lock(&(this->mutex));
  result = this->payload;
  pthread_mutex_unlock(&(this->mutex));
  return result;
}

std::string FetchAPI::getError() {
  std::string result;
  pthread_mutex_lock(&(this->mutex));
  result = this->errorMsg;
  pthread_mutex_unlock(&(this->mutex));
  return result;
}

FetchAPI::FetchErrorCode_t FetchAPI::getErrorCode() {
  FetchAPI::FetchErrorCode_t result;
  pthread_mutex_lock(&(this->mutex));
  result = this->errorCode;
  pthread_mutex_unlock(&(this->mutex));
  return result;
}