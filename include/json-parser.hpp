#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include <string>
#include <sstream>
#include <vector>
#include "json.hpp"

typedef nlohmann::ordered_json JsonBuilder;

class JsonObject {
private:
    nlohmann::json root;

    const nlohmann::json* getJsonObjectFromPath(const std::string& path) const;
    static std::vector<std::string> split(const std::string& s, const std::string& delimiter);


public:
    JsonObject(const std::string& json);

    ~JsonObject();

    class JsonValue {
    private:
        const nlohmann::json* value;

    public:
        JsonValue(const nlohmann::json& val);
        JsonValue();
        bool isAvailable() const;
        std::string getString() const;
        std::string getType() const;
        int getArraySize() const;
    };

    JsonValue operator[](const std::string& path) const;
};

#endif
