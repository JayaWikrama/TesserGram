#include "json-parser.hpp"

JsonObject::JsonObject() {
    // do nothing
}

JsonObject::JsonObject(const std::string& json) {
    root = nlohmann::json::parse(json, nullptr, false);
}

JsonObject::~JsonObject(){
    
}

void JsonObject::parse(const std::string& json) {
    root = nlohmann::json::parse(json, nullptr, false);
}

std::vector<std::string> JsonObject::split(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = s.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
        end = s.find(delimiter, start);
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

const nlohmann::json* JsonObject::getJsonObjectFromPath(const std::string& path) const {
    std::vector<std::string> segments = split(path, "->");
    const nlohmann::json* current = &root;

    for (const auto& segment : segments) {
        if (!current) return nullptr;

        size_t bracket_pos = segment.find('[');
        if (bracket_pos != std::string::npos) {
            std::string key = segment.substr(0, bracket_pos);
            size_t bracket_end = segment.find(']', bracket_pos);
            if (bracket_end == std::string::npos) return nullptr;

            int index = std::stoi(segment.substr(bracket_pos + 1, bracket_end - bracket_pos - 1));

            if (!current->is_object() || !current->contains(key)) return nullptr;
            current = &(*current)[key];

            if (!current->is_array() || index < 0 || index >= current->size()) return nullptr;
            current = &(*current)[index];
        } else {
            if (!current->is_object() || !current->contains(segment)) return nullptr;
            current = &(*current)[segment];
        }
    }
    return current;
}

JsonObject::JsonValue::JsonValue(const nlohmann::json& val) : value(&val) {}
JsonObject::JsonValue::JsonValue() : value(nullptr) {}

bool JsonObject::JsonValue::isAvailable() const {
    return value != nullptr;
}

std::string JsonObject::JsonValue::getString() const {
    if (!value) return "";
    if (value->is_string()) return value->get<std::string>();
    return value->dump();
}

std::string JsonObject::JsonValue::getType() const {
    if (!value) return "null";
    if (value->is_null()) return "null";
    if (value->is_boolean()) return "boolean";
    if (value->is_number_float()) return "double";
    if (value->is_number_integer()) return "int";
    if (value->is_object()) return "object";
    if (value->is_array()) return "array";
    if (value->is_string()) return "string";
    return "unknown";
}

int JsonObject::JsonValue::getArraySize() const {
    if (!value || !value->is_array()) return 0;
    return static_cast<int>(value->size());
}

JsonObject::JsonValue JsonObject::operator[](const std::string& path) const {
    const auto* val = getJsonObjectFromPath(path);
    return val ? JsonValue(*val) : JsonValue();
}