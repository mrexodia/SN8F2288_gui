#ifndef DISASSEMBLERBACKEND_H
#define DISASSEMBLERBACKEND_H

#include <QString>
#include <QStringList>

#include <vector>

#include "Core.h"
#include "Token.h"
#include "Database.h"

class DisassemblerBackend
{
public:
    DisassemblerBackend();

    std::vector<std::vector<Token>> tokenLines;

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
        std::vector<uint16_t> referencedFrom;

        //QString label;
        InstrEntry entry;
        Operand operands[2] = { Operand(), Operand() };
        //QString comment;

        std::vector<Token> toTokens(const Database & db) const
        {
            std::vector<Token> line;
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
                    auto tokOperand = [&](const Operand & op) -> Token
                    {
                        switch(op.type)
                        {
                        case Operand::Reg:
                            return Token::op(Token::Reg, op.text, op.value, op.bit);
                        case Operand::Imm:
                            return Token::op(Token::Imm, op.text, op.value, op.bit);
                        case Operand::Ram:
                        {
                            auto label = db.findLocalRamLabelByAddr(addr, op.value);
                            if(label.isEmpty())
                                label = db.findGlobalRamLabelByAddr(op.value);
                            return Token::op(Token::Ram, label.isEmpty() ? op.text : label, op.value, op.bit);
                        }
                        case Operand::Bit:
                        {
                            auto label = db.findLocalRamBitLabelByAddr(addr, op.value, op.bit);
                            if(label.isEmpty())
                                label = db.findGlobalRamBitLabelByAddr(op.value, op.bit);
                            if(label.isEmpty())
                            {
                                auto ram = db.findLocalRamLabelByAddr(addr, op.value);
                                if(ram.isEmpty())
                                    ram = db.findGlobalRamLabelByAddr(op.value);
                                if(!ram.isEmpty())
                                    label = QString("%1.%2").arg(ram).arg(op.bit);
                            }
                            return Token::op(Token::Bit, label.isEmpty() ? op.text : label, op.value, op.bit);
                        }
                        case Operand::Rom:
                        {
                            auto label = db.findRomLabelByAddr(op.value);
                            return Token::op(Token::Rom, label.isEmpty() ? op.text : label, op.value, op.bit);
                        }
                        }
                    };
                    line.push_back(tokOperand(operands[0]));
                    if(QString(entry.mnemonic) == QString("JMP") && operands[0].type == Operand::Rom) //CALL/JMP
                    {
                        auto arrow = operands[0].value < addr ? QChar(0x25B2) : QString();
                        auto diff = operands[0].value - addr;
                        if(qAbs(diff) < 10)
                            arrow += QString(" $%1%2").arg(diff > 0 ? "+" : "").arg(diff);
                        line.push_back(Token::other(QString(arrow)));
                    }
                    else if(operands[1].valid)
                    {
                        line.push_back(Token::comma());
                        line.push_back(Token::space());
                        line.push_back(tokOperand(operands[1]));
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
    std::vector<DisasmLine> lineInfo;

    bool disassemble(DisasmLine & line, std::vector<uint16_t> & branches) const;
    void loadRom(const QString & file);
};

#endif // DISASSEMBLERBACKEND_H
