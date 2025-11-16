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

    class TKeyButton
    {
    public:
        typedef enum _TValueType_t
        {
            COMMON = 0,
            URL = 1,
            CALLBACK_QUERY = 2
        } TValueType_t;

        TKeyButton(TValueType_t type, const std::string &text, const std::string &value);
        ~TKeyButton();
        TValueType_t getType() const;
        const std::string &getText() const;
        const std::string &getValue() const;

    private:
        TValueType_t type;
        std::string text;
        std::string value;
    };

    TKeyboard(TKeyType_t type, const std::string &caption);
    ~TKeyboard();

    TKeyboard &add(const std::string &button);
    TKeyboard &add(TKeyButton::TValueType_t type, const std::string &text, const std::string &value);
    TKeyboard &add(const TKeyButton &button);
    TKeyboard &add(const std::vector<TKeyButton> &buttons);

    const std::string &getCaption() const;
    std::string getMarkup() const;

private:
    TKeyType_t type;
    std::string caption;
    std::vector<std::string> buttons;

    bool addButton(const std::string &button);
    bool addButton(TKeyButton::TValueType_t type, const std::string &text, const std::string &value);
    bool addButton(const TKeyButton &button);
    bool addButton(const std::vector<TKeyButton> &buttons);
};

#endif