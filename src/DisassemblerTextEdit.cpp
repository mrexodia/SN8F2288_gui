#include "DisassemblerTextEdit.h"
#include <QKeyEvent>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextDocumentFragment>

#include "Utf8Ini/Utf8Ini.h"

DisassemblerTextEdit::DisassemblerTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    setWordWrapMode(QTextOption::NoWrap);
    mHighlighter = new DisassemblerHighlighter(document());
    mHighlighter->tokenLines = &mBackend.tokenLines;
    QPalette p;
    p.setColor(QPalette::Base, disassemblerBackground);
    setPalette(p);
}

std::vector<Token>* DisassemblerTextEdit::selectedLine() const
{
    auto addr = selectedAddr();
    if(addr < mHighlighter->tokenLines->size())
        return &mHighlighter->tokenLines->at(addr);
    return nullptr;
}

uint16_t DisassemblerTextEdit::selectedAddr() const
{
    return textCursor().blockNumber();
}

Token* DisassemblerTextEdit::selectedToken() const
{
    std::vector<Token>* line = selectedLine();
    if(line && !line->empty())
    {
        int column = textCursor().columnNumber();
        for(Token & token : *line)
        {
            if(token.type == Token::Space || token.type == Token::Comma)
                continue;
            if(column >= token.start && column <= token.start + token.text.length())
                return &token;
        }
        if(column == line->back().start + line->back().text.length())
            return &line->back();
    }
    return nullptr;
}

void DisassemblerTextEdit::loadCfg(const QString & file)
{
    //TODO: fix some duplicate code
    QFile f(file);
    if(!f.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Error", "Failed to open file...");
        return;
    }
    auto data = f.readAll().toStdString();
    Utf8Ini ini;
    int errorLine = -1;
    if(!ini.Deserialize(data, errorLine))
    {
        QMessageBox::warning(this, "Error", QString("Failed to deserialize INI (line %d)").arg(errorLine));
        return;
    }
    for(auto & section : ini.Sections())
    {
        auto qsection = QString::fromStdString(section);
        if(section == "chip")
        {
            //ignore
        }
        else if(section == "ram")
        {
            for(auto & key : ini.Keys(section))
            {
                bool ok = false;
                auto ramAddrText = QString::fromStdString(key);
                auto bitIdx = ramAddrText.indexOf('.');
                if(bitIdx != -1)
                {
                    bool ok1 = false, ok2 = false;
                    auto ramAddr = ramAddrText.left(bitIdx).toUShort(&ok1, 16);
                    auto bitAddr = ramAddrText.right(1).toUShort(&ok2);
                    if(ok1 && ok2)
                        mBackend.db.setGlobalRamBitLabel(ramAddr, bitAddr, QString::fromStdString(ini.GetValue(section, key)));
                    else
                        qDebug() << "bit nein";
                }
                else
                {
                    auto ramAddr = ramAddrText.toUShort(&ok, 16);
                    if(ok)
                        mBackend.db.setGlobalRamLabel(ramAddr, QString::fromStdString(ini.GetValue(section, key)));
                }
            }
        }
        else if(qsection.startsWith("ram@"))
        {
            auto midIdx = qsection.indexOf('-');
            if(midIdx != -1)
            {
                auto startText = qsection.mid(4, midIdx - 4);
                auto endText = qsection.mid(midIdx + 1);
                bool ok1 = false, ok2 = false;
                auto start = startText.toUShort(&ok1, 16);
                auto end = endText.toUShort(&ok2, 16);
                if(ok1 && ok2)
                {
                    auto romRange = mBackend.db.addRomRange(start, end);
                    for(auto & key : ini.Keys(section))
                    {
                        bool ok = false;
                        auto ramAddrText = QString::fromStdString(key);
                        auto bitIdx = ramAddrText.indexOf('.');
                        if(bitIdx != -1)
                        {
                            bool ok1 = false, ok2 = false;
                            auto ramAddr = ramAddrText.left(bitIdx).toUShort(&ok1, 16);
                            auto bitAddr = ramAddrText.right(1).toUShort(&ok2);
                            if(ok1 && ok2)
                                romRange->ramBitLabels.insert({ramAddr, bitAddr}, QString::fromStdString(ini.GetValue(section, key)));
                            else
                                qDebug() << "bit nein";
                        }
                        else
                        {
                            auto ramAddr = ramAddrText.toUShort(&ok, 16);
                            if(ok)
                                romRange->ramLabels.insert(ramAddr, QString::fromStdString(ini.GetValue(section, key)));
                        }
                    }
                }
                else
                {
                    QMessageBox::warning(this, "Error", QString("Incorrectly formatted section '%1'").arg(qsection));
                    return;
                }
            }
            else
            {
                QMessageBox::warning(this, "Error", QString("Incorrectly formatted section '%1'").arg(qsection));
                return;
            }
        }
        else if(section == "rom")
        {
            for(auto & key : ini.Keys(section))
            {
                bool ok = false;
                auto romAddr = QString(key.c_str()).toUShort(&ok, 16);
                if(ok)
                    mBackend.db.setRomLabel(romAddr, QString::fromStdString(ini.GetValue(section, key)));
            }
        }
        else if(section == "comment")
        {
            for(auto & key : ini.Keys(section))
            {
                bool ok = false;
                auto romAddr = QString(key.c_str()).toUShort(&ok, 16);
                if(ok)
                    mBackend.db.setRomComment(romAddr, QString::fromStdString(ini.GetValue(section, key)));
            }
        }
        else
        {
            qDebug() << "unsupported section" << section.c_str();
        }
    }
    refreshRom();
}

void DisassemblerTextEdit::keyPressEvent(QKeyEvent* event)
{
    qDebug() << "keyPressEvent, key: " << event->text();
    auto text = event->text();
    if(text.isEmpty()) //
        QPlainTextEdit::keyPressEvent(event);
    else if(text == "g") // Go to
    {
        QString curAddr = "1337";
        bool ok = false;
        QString addrText = QInputDialog::getText(this, "Go to", "ROM Address: ", QLineEdit::Normal, curAddr, &ok);
        if(ok && !addrText.isEmpty())
        {
            auto addr = addrText.toUShort(&ok, 16);
            if(ok)
            {
                QTextBlock block = document()->findBlockByLineNumber(addr);
                if(block.isValid())
                {
                    QTextCursor newCursor(block);
                    setTextCursor(newCursor);
                    return;
                }
            }
            QMessageBox::warning(this, "Error", QString("Invalid address %1").arg(addrText));
        }
    }
    else if(text == "n") // Label
    {
        auto addr = selectedAddr();
        auto t = selectedToken();
        if(t)
        {
            switch(t->type)
            {
            case Token::Label:
            case Token::Addr:
            case Token::Rom:
            {
                auto addr = t->value;
                QString label = mBackend.db.findRomLabelByAddr(addr);
                bool ok = false;
                auto title = QString().sprintf("ROM label at 0x%04x", addr);
                label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                if(ok)
                {
                    mBackend.db.setRomLabel(addr, label);
                    refreshRom();
                }
                break;
            }

            case Token::Ram:
            {
                auto range = mBackend.db.findRomRange(addr);
                if(range)
                {
                    auto found = range->ramLabels.find(t->value);
                    QString label = found != range->ramLabels.end() ? found.value() : QString();
                    bool ok = false;
                    auto title = QString().sprintf("Local RAM label at 0x%02x", t->value);
                    label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                    if(ok)
                    {
                        range->ramLabels[t->value] = label;
                        refreshRom();
                    }
                }
                else
                {
                    QString label = mBackend.db.findGlobalRamLabelByAddr(t->value);
                    bool ok = false;
                    auto title = QString().sprintf("Global RAM label at 0x%02x", t->value);
                    label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                    if(ok)
                    {
                        mBackend.db.setGlobalRamLabel(t->value, label);
                        refreshRom();
                    }
                }
                break;
            }

            case Token::Bit:
            {
                auto range = mBackend.db.findRomRange(addr);
                if(range)
                {
                    auto found = range->ramBitLabels.find({t->value, t->bit});
                    QString label = found != range->ramBitLabels.end() ? found.value() : QString();
                    bool ok = false;
                    auto title = QString().sprintf("Local RAM label at 0x%02x.%d", t->value, t->bit);
                    label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                    if(ok)
                    {
                        range->ramBitLabels[{t->value, t->bit}] = label;
                        refreshRom();
                    }
                }
                else
                {
                    QString label = mBackend.db.findGlobalRamBitLabelByAddr(t->value, t->bit);
                    bool ok = false;
                    auto title = QString().sprintf("Global RAM label at 0x%02x.%d", t->value, t->bit);
                    label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                    if(ok)
                    {
                        mBackend.db.setGlobalRamBitLabel(t->value, t->bit, label);
                        refreshRom();
                    }
                }
                break;
            }

            default:
                qDebug() << "unsupported rename! token start" << t->start << "token text" << t->text << "token addr" << t->value << "token type" << t->type << "bit" << t->bit;
            }
        }
    }
    else if(text == "h") // Highlight
    {
        auto token = selectedToken();
        if(token && !token->highlightMatch(mHighlighter->highlightToken))
            mHighlighter->highlightToken = *token;
        else
            mHighlighter->highlightToken = Token();
        mHighlighter->rehighlight();
    }
    else if(text == ";") // Comment
    {
        auto addr = selectedAddr();
        QString comment = mBackend.db.findRomCommentByAddr(addr);
        bool ok = false;
        auto title = QString().sprintf("Set comment at 0x%04x", addr);
        comment = QInputDialog::getText(this, title, title, QLineEdit::Normal, comment, &ok, Qt::Popup);
        if(ok)
        {
            mBackend.db.setRomComment(addr, comment);
            refreshRom();
        }
    }
}

void DisassemblerTextEdit::loadRom(const QString & file)
{
    mBackend.loadRom(file);
    refreshRom();
}

void DisassemblerTextEdit::refreshRom()
{
    auto firstVisible = firstVisibleBlock().blockNumber();
    auto oldBlock = textCursor().blockNumber();

    //do token conversion
    for(size_t i = 0; i < mBackend.lineInfo.size(); i++)
        mBackend.tokenLines[i] = mBackend.lineInfo[i].toTokens(mBackend.db);

    //remove nops from the end
    size_t displaySize = mBackend.lineInfo.size();
    while(displaySize && mBackend.lineInfo.at(displaySize - 1).data == 0)
        displaySize--;

    QString text;
    for(size_t i = 0; i < displaySize; i++)
    {
        if(!text.isEmpty())
            text.push_back('\n');
        for(const Token & t : mBackend.tokenLines.at(i))
            text += t.text;
    }
    document()->setPlainText(text);

    //https://stackoverflow.com/questions/27036048/how-to-scroll-to-the-specified-line-in-qplaintextedit
    moveCursor(QTextCursor::End);
    setTextCursor(QTextCursor(document()->findBlockByNumber(firstVisible)));
    setTextCursor(QTextCursor(document()->findBlockByNumber(oldBlock)));

    mHighlighter->pc = 8; //TODO: signal
    mHighlighter->rehighlight();
}
