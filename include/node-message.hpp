#ifndef __NODE_MESSAGE_HPP__
#define __NODE_MESSAGE_HPP__

#include <functional>
#include "type.hpp"
#include "utils/include/nlohmann/json_fwd.hpp"

class NodeMessage
{
private:
    long long updateId;
    CallbackQuery callbackQuery;
    Message message;

public:
    NodeMessage();
    NodeMessage(const nlohmann::json &message);
    ~NodeMessage();

    void parse(const nlohmann::json &message);
    void display() const;

    long long getId() const;

    const NodeMessage &processMessage(std::function<void(const Message &)> handler) const;
    const NodeMessage &processCallbackQuery(std::function<void(const CallbackQuery &)> handler) const;
};

#endif