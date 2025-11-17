#ifndef __FETCH_API_HPP__
#define __FETCH_API_HPP__

#include <string>
#include <vector>
#include <map>
#include <mutex>
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
    FETCH_ERR_UNKNOWN = 0x09
  };

private:
  std::string url;
  unsigned short connectTimeout;
  unsigned short totalTimeout;
  std::string payload;
  std::string errorMsg;
  ReturnCode code;
  std::map<std::string, std::string> headers;
  std::string body;
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

  void setHiddenConfidential(const std::string &confidential);
  void setHiddenConfidential(const std::vector<std::string> &confidential);

  void insertHeader(const std::string &key, const std::string &value);
  void setBody(const std::string &data);

  bool get(const std::map<std::string, std::string> &params = {});
  bool post();
  bool sendFile();
  bool download(std::vector<unsigned char> &data, const std::map<std::string, std::string> &params = {});

  std::string getPayload();
  std::string getError();
  ReturnCode getErrorCode();
};

#endif