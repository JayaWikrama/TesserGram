#ifndef __KEYBOARD_HPP__
#define __KEYBOARD_HPP__

#include <string>
#include <vector>

class TKeyboard
{
public:
    enum class Type : uint8_t
    {
        KEYBOARD = 0x00,
        INLINE_KEYBOARD = 0x01
    };

    class TKeyButton
    {
    public:
        enum class Type : uint8_t
        {
            COMMON = 0,
            URL = 1,
            CALLBACK_QUERY = 2
        };

        TKeyButton(Type type, const std::string &text, const std::string &value);
        ~TKeyButton();
        Type getType() const;
        const std::string &getText() const;
        const std::string &getValue() const;

    private:
        Type type;
        std::string text;
        std::string value;
    };

    TKeyboard(Type type, const std::string &caption);
    ~TKeyboard();

    TKeyboard &add(const std::string &button);
    TKeyboard &add(TKeyButton::Type type, const std::string &text, const std::string &value);
    TKeyboard &add(const TKeyButton &button);
    TKeyboard &add(const std::vector<TKeyButton> &buttons);

    const std::string &getCaption() const;
    std::string getMarkup() const;

private:
    Type type;
    std::string caption;
    std::vector<std::string> buttons;

    bool addButton(const std::string &button);
    bool addButton(TKeyButton::Type type, const std::string &text, const std::string &value);
    bool addButton(const TKeyButton &button);
    bool addButton(const std::vector<TKeyButton> &buttons);
};

#endif