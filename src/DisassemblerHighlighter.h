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

private:
    QMap<QString, QPair<QColor, QColor>> mnemonicMap;
    QColor highlightColor = "#FF0000";
    QColor pcForeground = "#FFFFFF", pcBackground = "#000000";
    QColor addrForeground = "#808080", addrBackground = Qt::transparent;
    QColor spaceForeground = "#000000", spaceBackground = Qt::transparent;
    QColor commaForeground = "#000000", commaBackground = Qt::transparent;
    QColor labelForeground = "#FF0000", labelBackground = Qt::transparent;
    QColor mnemonicForeground = "#000000", mnemonicBackground = Qt::transparent;
    QColor regForeground = "#008300", regBackground = Qt::transparent;
    QColor immForeground = "#828200", immBackground = Qt::transparent;
    QColor ramForeground = "#B30059", ramBackground = Qt::transparent;
    QColor bitForeground = "#B30059", bitBackground = Qt::transparent;
    QColor romForeground = "#000000", romBackground = "#FFFF00";
    QColor commentForeground = "#1E7A00", commentBackground = Qt::transparent;
    QColor otherForeground = "#000000", otherBackground = Qt::transparent;
};
