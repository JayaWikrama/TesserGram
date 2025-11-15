#include "function.hpp"

Function::Function() : param(nullptr), func(nullptr) {}

Function::Function(CallbackFunc func, void *param) : param(param), func(func) {}

Function::~Function() {}

void Function::set(CallbackFunc func, void *param)
{
    this->func = func;
    this->param = param;
}

bool Function::ready() const
{
    return (this->func != nullptr);
}

CallbackFunc Function::getFunction() const
{
    return this->func;
}

void *Function::getParam() const
{
    return this->param;
}

void Function::exec(Telegram &dframe)
{
    this->func(dframe, this->param);
}

void Function::exec(Telegram &dframe, void *param)
{
    this->func(dframe, param);
}