#ifndef __NODE_MESSAGE_HPP__
#define __NODE_MESSAGE_HPP__

#include <functional>
#include "type.hpp"

class NodeMessage
{
private:
    long long updateId;
    CallbackQuery callbackQuery;
    Message message;

public:
    NodeMessage();
    NodeMessage(const std::string &message);
    ~NodeMessage();

    void parse(const std::string &message);
    void display() const;

    long long getId() const;

    const NodeMessage &processMessage(std::function<void(const Message &)> handler) const;
    const NodeMessage &processCallbackQuery(std::function<void(const CallbackQuery &)> handler) const;
};

#endif