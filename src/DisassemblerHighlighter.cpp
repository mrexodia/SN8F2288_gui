#include "DisassemblerHighlighter.h"
#include <QDebug>
#include <QApplication>
#include <QCoreApplication>
#include <QJsonDocument>

DisassemblerHighlighter::DisassemblerHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
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
                format.setUnderlineColor(highlightColor);
                format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            }
            switch(token.type)
            {
            case Token::Addr:
                if(addr == pc)
                {
                    format.setForeground(pcForeground);
                    format.setBackground(pcBackground);
                }
                else
                {
                    format.setForeground(addrForeground);
                    format.setBackground(addrBackground);
                }
                break;
            case Token::Space:
                format.setForeground(spaceForeground);
                format.setBackground(spaceBackground);
                break;
            case Token::Comma:
                format.setForeground(commaForeground);
                format.setBackground(commaBackground);
                break;
            case Token::Label:
                format.setForeground(labelForeground);
                format.setBackground(labelBackground);
                break;
            case Token::Mnemonic:
            {
                auto found = mnemonicMap.find(token.text);
                if(found != mnemonicMap.end())
                {
                    format.setForeground(found.value().first);
                    format.setBackground(found.value().second);
                }
                else
                {
                    format.setForeground(mnemonicForeground);
                    format.setBackground(mnemonicBackground);
                }
                break;
            }
            case Token::Reg:
                format.setForeground(regForeground);
                format.setBackground(regBackground);
                break;
            case Token::Imm:
                format.setForeground(immForeground);
                format.setBackground(immBackground);
                break;
            case Token::Ram:
                if(token.value >= 0x80 && token.value <= 0xFF)
                {
                    format.setForeground(regForeground);
                    format.setBackground(regBackground);
                }
                else
                {
                    format.setForeground(ramForeground);
                    format.setBackground(ramBackground);
                }
                break;
            case Token::Bit:
                if(token.value >= 0x80 && token.value <= 0xFF)
                {
                    format.setForeground(regForeground);
                    format.setBackground(regBackground);
                }
                else
                {
                    format.setForeground(bitForeground);
                    format.setBackground(bitBackground);
                }
                break;
            case Token::Rom:
                format.setForeground(romForeground);
                format.setBackground(romBackground);
                break;
            case Token::Comment:
                format.setForeground(commentForeground);
                format.setBackground(commentBackground);
                break;
            case Token::Other:
                format.setForeground(otherForeground);
                format.setBackground(otherBackground);
                break;
            }
            setFormat(token.start, token.text.length(), format);
        }
    }
}
