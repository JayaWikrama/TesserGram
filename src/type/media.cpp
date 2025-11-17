#include <array>
#include "type.hpp"
#include "nlohmann/json.hpp"
#include "json-validator.hpp"
#include "utils/include/debug.hpp"

namespace
{
    static const std::array<std::string, 10> mediaNames = {
        "document",
        "photo",
        "animation",
        "sticker",
        "story",
        "video",
        "video_note",
        "voice",
        "audio",
        "contact"};

    static const std::string unknownName = "unknown";
}

Media::Media()
{
    this->type = Media::Type::DOCUMENT;
    this->fileSize = 0;
}

Media::~Media()
{
}

bool Media::empty() const
{
    return this->fileId.empty();
}

bool Media::parse(Media::Type type, const nlohmann::json &json)
{
    try
    {
        JSONValidator jvalidator(__FILE__, __LINE__, __func__);

        this->type = type;

        if (json.contains("file_size"))
            this->fileSize = jvalidator.get<long long>(json, "file_size");
        else
            this->fileSize = 0;

        this->fileId = jvalidator.get<std::string>(json, "file_id");
        this->fileUniqueId = jvalidator.get<std::string>(json, "file_unique_id");

        if (json.contains("file_name"))
            this->fileName = jvalidator.get<std::string>(json, "file_name");

        return true;
    }
    catch (const std::exception &e)
    {
        Debug::log(Debug::ERROR, __FILE__, __LINE__, __func__, "parse failed: %s\n", e.what());
    }
    this->reset();
    return false;
}

void Media::reset()
{
    this->type = Media::Type::DOCUMENT;
    this->fileSize = 0;
    this->fileId.clear();
    this->fileUniqueId.clear();
    this->fileName.clear();
}

const std::string Media::getType() const
{
    return this->typeToString(this->type);
}

const std::string &Media::typeToString(const Media::Type &type)
{
    std::size_t index = static_cast<std::size_t>(type);

    if (index < mediaNames.size())
    {
        return mediaNames[index];
    }
    return unknownName;
}

void Media::typeIteration(std::function<void(const Type &, const std::string &)> handler)
{
    uint8_t idx = 0;
    for (const std::string &name : mediaNames)
    {
        handler(static_cast<Media::Type>(idx), name);
        idx++;
    }
}