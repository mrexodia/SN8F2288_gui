#ifndef DISASSEMBLERBACKEND_H
#define DISASSEMBLERBACKEND_H

#include <QString>
#include <QStringList>

#include "sn8f2288_chip.h"
#include "Token.h"
#include "Database.h"

class DisassemblerBackend
{
public:
    DisassemblerBackend();

    Database db;
    vector<vector<Token>> tokenLines;

    struct Operand
    {
        enum Type
        {
            Reg,
            Imm,
            Ram,
            Bit,
            Rom,
        } type;

        bool valid = false;
        uint16_t value = 0;
        uint8_t bit = 0;
        QString text;
    };

    struct DisasmLine
    {
        bool valid = false;
        uint16_t addr = 0;
        uint16_t data = 0;

        //QString label;
        InstrEntry entry;
        Operand operands[2] = { Operand(), Operand() };
        //QString comment;

        vector<Token> toTokens(const Database & db) const
        {
            vector<Token> line;
            line.push_back(Token::addr(addr));
            line.push_back(Token::space());
            QString label = db.findRomLabelByAddr(addr);
            if(!label.isEmpty())
            {
                line.push_back(Token::label(QString("<%1>").arg(label), addr));
                line.push_back(Token::space());
            }
            if(!valid)
            {
                line.push_back(Token::mnemonic("DW"));
                line.push_back(Token::space());
                line.push_back(Token::other(QString().sprintf("0x%04x", data)));
            }
            else
            {
                line.push_back(Token::mnemonic(entry.mnemonic));
                if(operands[0].valid)
                {
                    line.push_back(Token::space());
                    auto tokOperand = [&db](const Operand & op) -> Token
                    {
                        switch(op.type)
                        {
                        case Operand::Reg:
                            return Token::op(Token::Reg, op.text, op.value, op.bit);
                        case Operand::Imm:
                            return Token::op(Token::Imm, op.text, op.value, op.bit);
                        case Operand::Ram:
                            return Token::op(Token::Ram, op.text, op.value, op.bit);
                        case Operand::Bit:
                            return Token::op(Token::Bit, op.text, op.value, op.bit);
                        case Operand::Rom:
                        {
                            auto label = db.findRomLabelByAddr(op.value);
                            return Token::op(Token::Rom, label.isEmpty() ? op.text : label, op.value, op.bit);
                        }
                        }
                    };
                    line.push_back(tokOperand(operands[0]));
                    if(operands[1].valid)
                    {
                        line.push_back(Token::comma());
                        line.push_back(Token::space());
                        line.push_back(tokOperand(operands[0]));
                    }
                }
            }

            auto comment = db.findRomCommentByAddr(addr);
            if(!comment.isEmpty())
            {
                line.push_back(Token::space());
                line.push_back(Token::other(";"));
                line.push_back(Token::space());
                line.push_back(Token::comment(comment));
            }

            size_t start = 0;
            for(Token & t : line)
            {
                t.start = start;
                start += t.text.length();
            }

            return line;
        }
    };
    vector<DisasmLine> lineInfo;

    bool disassemble(DisasmLine & line, vector<uint16_t> & branches) const;
    void loadRom(const QString & file);

    SN8F2288 chip;
};

#endif // DISASSEMBLERBACKEND_H
