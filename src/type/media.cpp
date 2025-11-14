#include "type.hpp"
#include "nlohmann/json.hpp"
#include "json-validator.hpp"
#include "utils/include/debug.hpp"

const MediaTypeEntry mediaTypes[10] = {
    {Media::DOCUMENT, "document"},
    {Media::PHOTO, "photo"},
    {Media::ANIMATION, "animation"},
    {Media::STICKER, "sticker"},
    {Media::STORY, "story"},
    {Media::VIDEO, "video"},
    {Media::VIDEO_NOTE, "video_note"},
    {Media::VOICE, "voice"},
    {Media::AUDIO, "audio"},
    {Media::CONTACT, "contact"}};

Media::Media()
{
    this->type = Media::DOCUMENT;
    this->fileSize = 0;
}

Media::~Media()
{
}

bool Media::parse(TYPE_t type, const nlohmann::json &json)
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
    this->type = Media::DOCUMENT;
    this->fileSize = 0;
    this->fileId.clear();
    this->fileUniqueId.clear();
    this->fileName.clear();
}

const std::string Media::getType()
{
    for (const auto &entry : mediaTypes)
    {
        if (entry.type == this->type)
            return std::string(entry.key);
    }
    return "unknown";
}
