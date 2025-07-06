#ifndef __FETCH_API_HPP__
#define __FETCH_API_HPP__

#include <string>
#include <vector>
#include <map>
#include <pthread.h>
#include "debug.hpp"

class FetchAPI {
  public:
    typedef enum __FetchErrorCode_t {
      FETCH_OK = 0,
      FETCH_ERR_INVALID_URL,
      FETCH_ERR_TIMEOUT,
      FETCH_ERR_CONNECTION,
      FETCH_ERR_SEND_FAILED,
      FETCH_ERR_RECV_FAILED,
      FETCH_ERR_HTTP_ERROR,
      FETCH_ERR_PARSE_RESPONSE,
      FETCH_ERR_UNSUPPORTED_PROTOCOL,
      FETCH_ERR_UNKNOWN
    } FetchErrorCode_t;

  private:
    std::string url;
    unsigned short connectTimeout;
    unsigned short totalTimeout;
    std::string payload;
    std::string errorMsg;
    FetchErrorCode_t errorCode;
    std::map<std::string, std::string> headers;
    std::string body;
    pthread_mutex_t mutex;

    Debug *debug;

    static size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *s);
    static size_t downloadCallback(void *contents, size_t size, size_t nmemb, std::vector<unsigned char> *v);
    void reset();

  public:
    FetchAPI(const std::string &url, unsigned short connectTimeout = 3, unsigned short totalTimeout = 5);
    ~FetchAPI();

    void enableDebug();
    void enableDebug(const std::string &confidential);
    void enableDebug(const std::vector <std::string> &confidential);
    void disableDebug();
    bool isDebugEnable();
    Debug& DEBUG();

    void insertHeader(const std::string &key, const std::string &value);
    void setBody(const std::string &data);

    bool get(const std::map<std::string, std::string> &params = {});
    bool post();
    bool sendFile();
    bool download(std::vector<unsigned char> &data, const std::map<std::string, std::string> &params = {});

    std::string getPayload();
    std::string getError();
    FetchErrorCode_t getErrorCode();
};

#endif