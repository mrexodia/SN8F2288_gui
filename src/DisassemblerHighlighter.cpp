#include "DisassemblerHighlighter.h"
#include <QDebug>
#include <QApplication>
#include <QCoreApplication>
#include <QJsonDocument>
#include "Config.h"

DisassemblerHighlighter::DisassemblerHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    auto & mnemonicMap = Config().mnemonicMap;
    mnemonicMap["RETI"] = mnemonicMap["RET"] = mnemonicMap["CALL"] = { "#000000", "#00FFFF" };
    mnemonicMap["JMP"] = { "#000000", "#FFFF00"};
    mnemonicMap["CMPRS"] = mnemonicMap["INCS"] = mnemonicMap["INCMS"] = mnemonicMap["DECS"] = mnemonicMap["DECMS"] = mnemonicMap["BTS0"] = mnemonicMap["BTS1"] = mnemonicMap["B0BTS0"] = mnemonicMap["B0BTS1"] = {"#FF0000", "#FFFF00"};
}

void DisassemblerHighlighter::highlightBlock(const QString & text)
{
    if(text.startsWith("0x"))
    {
        auto addr = text.left(6).toUShort(nullptr, 16);
        if(addr >= tokenLines->size())
            return;
        const std::vector<Token> & line = tokenLines->at(addr);

        for(const Token & token : line)
        {
            QTextCharFormat format;
            if(token.highlightMatch(highlightToken))
            {
                format.setUnderlineColor(Config().highlightColor);
                format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            }
            switch(token.type)
            {
            case Token::Addr:
                if(addr == pc)
                {
                    format.setForeground(Config().pcForeground);
                    format.setBackground(Config().pcBackground);
                }
                else
                {
                    format.setForeground(Config().addrForeground);
                    format.setBackground(Config().addrBackground);
                }
                break;
            case Token::Space:
                format.setForeground(Config().spaceForeground);
                format.setBackground(Config().spaceBackground);
                break;
            case Token::Comma:
                format.setForeground(Config().commaForeground);
                format.setBackground(Config().commaBackground);
                break;
            case Token::Label:
                format.setForeground(Config().labelForeground);
                format.setBackground(Config().labelBackground);
                break;
            case Token::Mnemonic:
            {
                auto found = Config().mnemonicMap.find(token.text);
                if(found != Config().mnemonicMap.end())
                {
                    format.setForeground(found.value().first);
                    format.setBackground(found.value().second);
                }
                else
                {
                    format.setForeground(Config().mnemonicForeground);
                    format.setBackground(Config().mnemonicBackground);
                }
                break;
            }
            case Token::Reg:
                format.setForeground(Config().regForeground);
                format.setBackground(Config().regBackground);
                break;
            case Token::Imm:
                format.setForeground(Config().immForeground);
                format.setBackground(Config().immBackground);
                break;
            case Token::Ram:
                if(token.value >= 0x80 && token.value <= 0xFF)
                {
                    format.setForeground(Config().regForeground);
                    format.setBackground(Config().regBackground);
                }
                else
                {
                    format.setForeground(Config().ramForeground);
                    format.setBackground(Config().ramBackground);
                }
                break;
            case Token::Bit:
                if(token.value >= 0x80 && token.value <= 0xFF)
                {
                    format.setForeground(Config().regForeground);
                    format.setBackground(Config().regBackground);
                }
                else
                {
                    format.setForeground(Config().bitForeground);
                    format.setBackground(Config().bitBackground);
                }
                break;
            case Token::Rom:
                format.setForeground(Config().romForeground);
                format.setBackground(Config().romBackground);
                break;
            case Token::Comment:
                format.setForeground(Config().commentForeground);
                format.setBackground(Config().commentBackground);
                break;
            case Token::Other:
                format.setForeground(Config().otherForeground);
                format.setBackground(Config().otherBackground);
                break;
            }
            setFormat(token.start, token.text.length(), format);
        }
    }
}
