#ifndef __JSON_VALIDATOR__
#define __JSON_VALIDATOR__

#include "nlohmann/json_fwd.hpp"

#include <functional>

class JSONValidator
{
private:
    enum class ReturnCode : uint8_t
    {
        OK = 0x00,
        NOT_FOUND = 0x01,
        TYPE_INVALID = 0x02,
        NOT_SET = 0x03,
        EMPTY = 0x04
    };

    ReturnCode code;
    int line;
    std::string src;
    std::string func;
    std::string err;
    const nlohmann::json *jval;

    template <typename T>
    const nlohmann::json &_validate(const nlohmann::json &json,
                                    const std::string &key,
                                    const std::string &parrentKey = "");

public:
    JSONValidator(const std::string &src, int line, const std::string &func);
    ~JSONValidator();

    void param(const std::string &src, int line, const std::string &func);
    const nlohmann::json &getObject(const nlohmann::json &json,
                                    const std::string &key,
                                    const std::string &parrentKey = "");
    const nlohmann::json &getArray(const nlohmann::json &json,
                                   const std::string &key,
                                   const std::string &parrentKey = "");

    template <typename T>
    T get(const nlohmann::json &json,
          const std::string &key,
          const std::string &parrentKey = "");

    template <typename T>
    JSONValidator &validate(const nlohmann::json &json,
                            const std::string &key,
                            const std::string &parrentKey = "");
    JSONValidator &object(const nlohmann::json &json,
                          const std::string &key,
                          const std::string &parrentKey = "");
    JSONValidator &array(const nlohmann::json &json,
                         const std::string &key,
                         const std::string &parrentKey = "");
    JSONValidator &onValid(std::function<void(const nlohmann::json &)> handler);
    JSONValidator &onNotFound(std::function<void(const nlohmann::json &, const std::string &err)> handler);
    JSONValidator &onTypeInvalid(std::function<void(const nlohmann::json &, const std::string &err)> handler);
    JSONValidator &onInvalid(std::function<void(const nlohmann::json &, const std::string &err)> handler);
    JSONValidator &onInvalid(std::function<void(const std::string &err)> handler);
    JSONValidator &onInvalid(std::function<void()> handler);
    void throwError();
};

#endif