#ifndef __KEYBOARD_HPP__
#define __KEYBOARD_HPP__

#include <string>
#include <vector>

class TKeyboard
{
public:
    typedef enum _TKeyType_t
    {
        KEYBOARD = 0,
        INLINE_KEYBOARD = 1
    } TKeyType_t;

    typedef enum _TValueType_t
    {
        COMMON = 0,
        URL = 1,
        CALLBACK_QUERY = 2
    } TValueType_t;

    typedef struct _TKeyButtonConstructor_t
    {
        TValueType_t type;
        std::string text;
        std::string value;
    } TKeyButtonConstructor_t;

    TKeyboard(TKeyType_t type, const std::string &caption);
    ~TKeyboard();

    TKeyboard &add(const std::string &button);
    TKeyboard &add(TValueType_t type, const std::string &text, const std::string &value);
    TKeyboard &add(const TKeyButtonConstructor_t &button);
    TKeyboard &add(const std::vector<TKeyButtonConstructor_t> &buttons);

    const std::string &getCaption() const;
    std::string getMarkup() const;

private:
    TKeyType_t type;
    std::string caption;
    std::vector<std::string> buttons;

    bool addButton(const std::string &button);
    bool addButton(TValueType_t type, const std::string &text, const std::string &value);
    bool addButton(const TKeyButtonConstructor_t &button);
    bool addButton(const std::vector<TKeyButtonConstructor_t> &buttons);
};

#endif