#pragma once

#include <QSyntaxHighlighter>
#include "Token.h"
#include <QString>
#include <vector>
#include <map>

class DisassemblerHighlighter : public QSyntaxHighlighter
{
public:
    DisassemblerHighlighter(QTextDocument* parent);

    std::vector<std::vector<Token>>* tokenLines;
    Token highlightToken;
    uint16_t pc = 0xffff;

protected:
    void highlightBlock(const QString &) override;
};
