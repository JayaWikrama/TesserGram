#include <type_traits>
#include "json-validator.hpp"
#include "utils/include/error.hpp"

#include "nlohmann/json.hpp"

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

static const char *getTypeString(nlohmann::json::value_t expectedType)
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

template <typename T>
const nlohmann::json &JSONValidator::_validate(const nlohmann::json &json,
                                               const std::string &key,
                                               const std::string &parrentKey)
{
    auto it = json.find(key);
    nlohmann::json::value_t expectedType = JsonTypeTrait<T>::type;
    if (it == json.end())
        throw std::runtime_error(Error::fieldNotFound(this->src, this->line, this->func, key, parrentKey));
    if (it->type() != expectedType)
        throw std::runtime_error(Error::fieldTypeInvalid(this->src, this->line, this->func, getTypeString(expectedType), key, parrentKey));
    return *it;
}

JSONValidator::JSONValidator(const std::string &src, int line, const std::string &func) : code(ReturnCode::NOT_SET),
                                                                                          src(src),
                                                                                          line(line),
                                                                                          func(func),
                                                                                          err(),
                                                                                          jval(nullptr)
{
}

JSONValidator::~JSONValidator() {}

void JSONValidator::param(const std::string &src, int line, const std::string &func)
{
    this->src = src;
    this->line = line;
    this->func = func;
}

const nlohmann::json &JSONValidator::getObject(const nlohmann::json &json,
                                               const std::string &key,
                                               const std::string &parrentKey)
{
    auto it = json.find(key);
    if (it == json.end())
        throw std::runtime_error(Error::fieldNotFound(this->src, this->line, this->func, key, parrentKey));
    if (it->type() != nlohmann::json::value_t::object)
        throw std::runtime_error(Error::fieldTypeInvalid(this->src, this->line, this->func, "object", key, parrentKey));
    if (it->empty())
        throw std::runtime_error(Error::common(src, line, func, key + " empty"));
    return *it;
}

const nlohmann::json &JSONValidator::getArray(const nlohmann::json &json,
                                              const std::string &key,
                                              const std::string &parrentKey)
{
    auto it = json.find(key);
    if (it == json.end())
        throw std::runtime_error(Error::fieldNotFound(this->src, this->line, this->func, key, parrentKey));
    if (it->type() != nlohmann::json::value_t::array)
        throw std::runtime_error(Error::fieldTypeInvalid(this->src, this->line, this->func, "object", key, parrentKey));
    if (it->empty())
        throw std::runtime_error(Error::common(src, line, func, key + " empty"));
    return *it;
}

template <typename T>
T JSONValidator::get(const nlohmann::json &json,
                     const std::string &key,
                     const std::string &parrentKey)
{
    const nlohmann::json &j = this->_validate<T>(json, key, parrentKey);
    T value = j.get<T>();
    return value;
}

template <typename T>
JSONValidator &JSONValidator::validate(const nlohmann::json &json,
                                       const std::string &key,
                                       const std::string &parrentKey)
{
    auto it = json.find(key);
    if (it == json.end())
    {
        this->err = Error::fieldNotFound(this->src, this->line, this->func, key, parrentKey);
        this->code = ReturnCode::NOT_FOUND;
        this->jval = &json;
        return *this;
    }
    this->jval = &(*it);
    nlohmann::json::value_t expectedType = JsonTypeTrait<T>::type;
    if (it->type() != expectedType)
    {
        this->err = Error::fieldTypeInvalid(this->src, this->line, this->func, getTypeString(expectedType), key, parrentKey);
        this->code = ReturnCode::TYPE_INVALID;
        return *this;
    }
    this->code = ReturnCode::OK;
    return *this;
}

JSONValidator &JSONValidator::onValid(std::function<void(const nlohmann::json &)> handler)
{
    if (this->code == ReturnCode::OK)
        handler(*(this->jval));
    return *this;
}

JSONValidator &JSONValidator::onNotFound(std::function<void(const nlohmann::json &, const std::string &err)> handler)
{
    if (this->code == ReturnCode::NOT_FOUND)
        handler(*(this->jval), err);
    return *this;
}

JSONValidator &JSONValidator::onTypeInvalid(std::function<void(const nlohmann::json &, const std::string &err)> handler)
{
    if (this->code == ReturnCode::TYPE_INVALID)
        handler(*(this->jval), err);
    return *this;
}

void JSONValidator::throwError()
{
    if (this->err.empty())
        return;
    throw std::runtime_error(this->err);
}

template std::string JSONValidator::get<std::string>(const nlohmann::json &, const std::string &, const std::string &);
template bool JSONValidator::get<bool>(const nlohmann::json &, const std::string &, const std::string &);
template int JSONValidator::get<int>(const nlohmann::json &, const std::string &, const std::string &);
template unsigned int JSONValidator::get<unsigned int>(const nlohmann::json &, const std::string &, const std::string &);
template long JSONValidator::get<long>(const nlohmann::json &, const std::string &, const std::string &);
template unsigned long JSONValidator::get<unsigned long>(const nlohmann::json &, const std::string &, const std::string &);
template long long JSONValidator::get<long long>(const nlohmann::json &, const std::string &, const std::string &);
template unsigned long long JSONValidator::get<unsigned long long>(const nlohmann::json &, const std::string &, const std::string &);
template float JSONValidator::get<float>(const nlohmann::json &, const std::string &, const std::string &);
template double JSONValidator::get<double>(const nlohmann::json &, const std::string &, const std::string &);

template JSONValidator &JSONValidator::validate<std::string>(const nlohmann::json &, const std::string &, const std::string &);
template JSONValidator &JSONValidator::validate<bool>(const nlohmann::json &, const std::string &, const std::string &);
template JSONValidator &JSONValidator::validate<int>(const nlohmann::json &, const std::string &, const std::string &);
template JSONValidator &JSONValidator::validate<unsigned int>(const nlohmann::json &, const std::string &, const std::string &);
template JSONValidator &JSONValidator::validate<long>(const nlohmann::json &, const std::string &, const std::string &);
template JSONValidator &JSONValidator::validate<unsigned long>(const nlohmann::json &, const std::string &, const std::string &);
template JSONValidator &JSONValidator::validate<long long>(const nlohmann::json &, const std::string &, const std::string &);
template JSONValidator &JSONValidator::validate<unsigned long long>(const nlohmann::json &, const std::string &, const std::string &);
template JSONValidator &JSONValidator::validate<float>(const nlohmann::json &, const std::string &, const std::string &);
template JSONValidator &JSONValidator::validate<double>(const nlohmann::json &, const std::string &, const std::string &);