#ifndef __FETCH_API_HPP__
#define __FETCH_API_HPP__

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include "nlohmann/json_fwd.hpp"
#include "utils/include/debug.hpp"

class FetchAPI
{
public:
  enum class ReturnCode : uint8_t
  {
    FETCH_OK = 0x00,
    FETCH_ERR_INVALID_URL = 0x01,
    FETCH_ERR_TIMEOUT = 0x02,
    FETCH_ERR_CONNECTION = 0x03,
    FETCH_ERR_SEND_FAILED = 0x04,
    FETCH_ERR_RECV_FAILED = 0x05,
    FETCH_ERR_HTTP_ERROR = 0x06,
    FETCH_ERR_PARSE_RESPONSE = 0x07,
    FETCH_ERR_UNSUPPORTED_PROTOCOL = 0x08,
    FETCH_ERR_INIT_ERROR = 0x09,
    FETCH_ERR_EMPTY_BODY = 0x0A,
    FETCH_HAS_NOT_BEEN_DONE = 0x0B,
    FETCH_ERR_PARSE_MIME = 0x0C,
    FETCH_ERR_UNKNOWN = 0x0D
  };

private:
  std::string url;
  unsigned short connectTimeout;
  unsigned short totalTimeout;
  std::string payload;
  std::string errorMsg;
  ReturnCode code;
  std::map<std::string, std::string> headers;
  mutable std::mutex mutex;

  Debug debug;

  class Mime
  {
  private:
    bool file;
    std::string name;
    std::string data;
    std::string type;

  public:
    Mime();
    ~Mime();
    bool parse(const nlohmann::json &json);
    bool isFile() const;
    bool isContainsType() const;
    const std::string &getName() const;
    const std::string &getData() const;
    const std::string &getType() const;
  };

  static size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *s);
  static size_t downloadCallback(void *contents, size_t size, size_t nmemb, std::vector<unsigned char> *v);
  void reset();

public:
  FetchAPI(const std::string &url, unsigned short connectTimeout = 3, unsigned short totalTimeout = 5);
  ~FetchAPI();

  FetchAPI &confidential(const std::string &confidential);
  FetchAPI &confidential(const std::vector<std::string> &confidential);

  FetchAPI &header(const std::string &key, const std::string &value);

  FetchAPI &get(const std::map<std::string, std::string> &params = {});
  FetchAPI &post(const std::string &body);
  FetchAPI &upload(const std::string &mime);
  FetchAPI &download(std::vector<unsigned char> &data, const std::map<std::string, std::string> &params = {});

  FetchAPI &onSuccess(std::function<void(const std::string &)> handler);
  FetchAPI &onTimeout(std::function<void()> handler);
  FetchAPI &onError(std::function<void(ReturnCode, const std::string &)> handler);

  std::string getPayload();
  std::string getError();
  ReturnCode getErrorCode();
};

#endif