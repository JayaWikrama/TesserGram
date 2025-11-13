#ifndef __JSON_VALIDATOR__
#define __JSON_VALIDATOR__

#include "nlohmann/json.hpp"
#include "utils/include/error.hpp"

#include <type_traits>

template <typename T>
struct JsonTypeTrait;

template <>
struct JsonTypeTrait<std::string>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::string;
};

template <>
struct JsonTypeTrait<bool>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::boolean;
};

template <>
struct JsonTypeTrait<int>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::number_integer;
};

template <>
struct JsonTypeTrait<unsigned int>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::number_unsigned;
};

template <>
struct JsonTypeTrait<long>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::number_unsigned;
};

template <>
struct JsonTypeTrait<unsigned long>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::number_unsigned;
};

template <>
struct JsonTypeTrait<long long>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::number_unsigned;
};

template <>
struct JsonTypeTrait<unsigned long long>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::number_unsigned;
};

template <>
struct JsonTypeTrait<float>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::number_float;
};

template <>
struct JsonTypeTrait<double>
{
    static const nlohmann::json::value_t type = nlohmann::json::value_t::number_float;
};

class JSONValidator
{
private:
    int line;
    std::string src;
    std::string func;

    const char *getTypeString(nlohmann::json::value_t expectedType)
    {
        switch (expectedType)
        {
        case nlohmann::json::value_t::null:
            return "null";
        case nlohmann::json::value_t::object:
            return "object";
        case nlohmann::json::value_t::array:
            return "array";
        case nlohmann::json::value_t::string:
            return "string";
        case nlohmann::json::value_t::boolean:
            return "boolean";
        case nlohmann::json::value_t::number_integer:
            return "number (integer)";
        case nlohmann::json::value_t::number_unsigned:
            return "number (unsigned)";
        case nlohmann::json::value_t::number_float:
            return "number (float)";
        case nlohmann::json::value_t::discarded:
            return "discarded";
        default:
            return "unknown";
        }
    }

    inline const nlohmann::json &_validate(const nlohmann::json &json,
                                           const std::string &key,
                                           const nlohmann::json::value_t expectedType,
                                           const std::string &parrentKey = "")
    {
        auto it = json.find(key);
        if (it == json.end())
            throw std::runtime_error(Error::fieldNotFound(this->src, this->line, this->func, key, parrentKey));
        if (it->type() != expectedType)
            throw std::runtime_error(Error::fieldTypeInvalid(this->src, this->line, this->func, this->getTypeString(expectedType), key, parrentKey));
        return *it;
    }

public:
    JSONValidator(const std::string &src, int line, const std::string &func) : src(src), line(line), func(func)
    {
    }

    ~JSONValidator() {}

    inline void param(const std::string &src, int line, const std::string &func)
    {
        this->src = src;
        this->line = line;
        this->func = func;
    }

    inline const nlohmann::json &getObject(const nlohmann::json &json,
                                           const std::string &key,
                                           const std::string &parrentKey = "")
    {
        const nlohmann::json &j = this->_validate(json, key, nlohmann::json::value_t::object, parrentKey);
        if (j.size() == 0)
            throw std::runtime_error(Error::common(src, line, func, key + " empty"));
        return j;
    }

    inline const nlohmann::json &getArray(const nlohmann::json &json,
                                          const std::string &key,
                                          const std::string &parrentKey = "")
    {
        const nlohmann::json &j = this->_validate(json, key, nlohmann::json::value_t::array, parrentKey);
        if (j.size() == 0)
            throw std::runtime_error(Error::common(src, line, func, key + " empty"));
        return j;
    }

    template <typename T>
    T get(const nlohmann::json &json,
          const std::string &key,
          const std::string &parrentKey = "")
    {
        const nlohmann::json &j = this->_validate(json, key, JsonTypeTrait<T>::type, parrentKey);
        T value = j.get<T>();
        return value;
    }
};

#endif