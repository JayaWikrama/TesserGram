#ifndef __FUNCTION_HPP__
#define __FUNCTION_HPP__

#include <array>

class Telegram;

typedef void (*CallbackFunc)(Telegram &, void *);

class Function
{
private:
    void *param;
    CallbackFunc func;

public:
    Function();
    Function(CallbackFunc func, void *param = nullptr);
    ~Function();

    void set(CallbackFunc func, void *param = nullptr);
    CallbackFunc getFunction() const;
    void *getParam() const;
    bool ready() const;
    void exec(Telegram &dframe);
    void exec(Telegram &dframe, void *param);
};

#endif