#ifndef TOKENPARTY_H
#define TOKENPARTY_H

#include <QString>

struct Token
{
    bool valid = false;
    int start = 0;

    enum Type
    {
        Addr,
        Space,
        Comma,
        Label,
        Mnemonic,
        Reg,
        Imm,
        Ram,
        Bit,
        Rom,
        Comment,
        Other
    } type;

    uint16_t value = 0;
    uint8_t bit = 0;
    QString text;

    bool highlightMatch(const Token & h) const
    {
        if(!h.valid || !valid)
            return false;
        switch(h.type)
        {
        case Addr:
        case Label:
        case Rom:
            return h.value == value && (type == Addr || type == Label || type == Rom);
        case Mnemonic:
            return h.text == text && (type == Mnemonic || type == Other);
        case Reg:
            return h.text == text; //TODO: memory register addresses
        case Imm:
            return h.value == value && type == Imm;
        case Ram:
            return h.value == value && (type == Ram || type == Bit);
        default:
            return h.type == type && h.text == text;
        }
    }

    static Token addr(uint16_t addr)
    {
        Token t;
        t.valid = true;
        t.type = Addr;
        t.value = addr;
        t.text = QString().sprintf("0x%04x", addr);
        return t;
    }

    static Token space()
    {
        Token t;
        t.valid = true;
        t.type = Space;
        t.text = " ";
        return t;
    }

    static Token comma()
    {
        Token t;
        t.valid = true;
        t.type = Comma;
        t.text = ",";
        return t;
    }

    static Token label(const QString & text, uint16_t addr)
    {
        Token t;
        t.valid = true;
        t.type = Label;
        t.value = addr;
        t.text = text;
        return t;
    }

    static Token mnemonic(const QString & text)
    {
        Token t;
        t.valid = true;
        t.type = Mnemonic;
        t.text = text;
        return t;
    }

    static Token other(const QString & text)
    {
        Token t;
        t.valid = true;
        t.type = Other;
        t.text = text;
        return t;
    }

    static Token comment(const QString & text)
    {
        Token t;
        t.valid = true;
        t.type = Comment;
        t.text = text;
        return t;
    }

    static Token op(Type type, const QString & text, uint16_t value, uint8_t bit)
    {
        Token t;
        t.valid = true;
        t.type = type;
        t.value = value;
        t.text = text;
        t.bit = bit;
        return t;
    }
};

#endif // TOKENPARTY_H
