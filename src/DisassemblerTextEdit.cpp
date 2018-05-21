#include "DisassemblerTextEdit.h"
#include <QKeyEvent>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextDocumentFragment>

#include "XrefsDialog.h"

DisassemblerTextEdit::DisassemblerTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    setWordWrapMode(QTextOption::NoWrap);
    mHighlighter = new DisassemblerHighlighter(document());
    mHighlighter->tokenLines = &mBackend.tokenLines;
    QPalette p;
    p.setColor(QPalette::Base, disassemblerBackground);
    setPalette(p);
    mXrefsDialog = new XrefsDialog(this);
    connect(mXrefsDialog, SIGNAL(gotoAddress(uint16_t)), this, SLOT(gotoAddress(uint16_t)));
    connect(mXrefsDialog, SIGNAL(rejected()), this, SLOT(xrefsRejectedSlot()));
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

bool DisassemblerTextEdit::gotoAddress(uint16_t addr)
{
    auto diff = qAbs(selectedAddr() - addr);
    QTextBlock block = document()->findBlockByLineNumber(addr);
    if(!block.isValid())
        return false;

    if(diff > 15)
    {
        moveCursor(QTextCursor::End);
        setTextCursor(QTextCursor(block));
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, 10);
        setTextCursor(cursor);
    }
    setTextCursor(QTextCursor(block));
    return true;
}

void DisassemblerTextEdit::xrefsRejectedSlot()
{
    gotoAddress(mOldXrefsAddr);
}

bool DisassemblerTextEdit::loadCfg(const QString & file)
{
    auto res = QMessageBox::question(this, "Clear database?", "Should I clear the previous database?\n\nProgress may be lost...", QMessageBox::Yes, QMessageBox::No);
    if(res == QMessageBox::Yes)
        Core::db().clear();
    if(!Core::db().load(file))
    {
        QMessageBox::warning(this, "Error", "Failed to load database...");
        return false;
    }
    QMessageBox::information(this, "Ok", "Database loaded!");
    refreshRom();
    return true;
}

bool DisassemblerTextEdit::saveCfg(const QString &file)
{
    if(!Core::db().save(file))
    {
        QMessageBox::warning(this, "Error", "Failed to save database...");
        return false;
    }
    QMessageBox::information(this, "Ok", "Database saved!");
    return true;
}

void DisassemblerTextEdit::keyPressEvent(QKeyEvent* event)
{
    qDebug() << "keyPressEvent, key: " << event->text();
    auto text = event->text();
    if(text.isEmpty()) //
        QPlainTextEdit::keyPressEvent(event);
    else if(text == "g") // Go to
    {
        QString curAddr = QString().sprintf("0x%04x", selectedAddr());
        bool ok = false;
        QString addrText = QInputDialog::getText(this, "Go to", "ROM Address: ", QLineEdit::Normal, curAddr, &ok);
        if(ok && !addrText.isEmpty())
        {
            auto addr = addrText.toUShort(&ok, 16);
            if(!ok || !gotoAddress(addr))
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
                QString label = Core::db().findRomLabelByAddr(addr);
                bool ok = false;
                auto title = QString().sprintf("ROM label at 0x%04x", addr);
                label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                if(ok)
                {
                    Core::db().setRomLabel(addr, label);
                    refreshRom();
                }
                break;
            }

            case Token::Ram:
            {
                auto range = Core::db().findRomRangeConst(addr);
                if(range)
                {
                    auto found = range->ramLabels.find(t->value);
                    QString label = found != range->ramLabels.end() ? found.value() : QString();
                    bool ok = false;
                    auto title = QString().sprintf("Local RAM label at 0x%02x", t->value);
                    label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                    if(ok)
                    {
                        Core::db().setLocalRamLabel(addr, t->value, label);
                        refreshRom();
                    }
                }
                else
                {
                    QString label = Core::db().findGlobalRamLabelByAddr(t->value);
                    bool ok = false;
                    auto title = QString().sprintf("Global RAM label at 0x%02x", t->value);
                    label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                    if(ok)
                    {
                        Core::db().setGlobalRamLabel(t->value, label);
                        refreshRom();
                    }
                }
                break;
            }

            case Token::Bit:
            {
                auto range = Core::db().findRomRangeConst(addr);
                if(range)
                {
                    auto found = range->ramBitLabels.find({t->value, t->bit});
                    QString label = found != range->ramBitLabels.end() ? found.value() : QString();
                    bool ok = false;
                    auto title = QString().sprintf("Local RAM label at 0x%02x.%d", t->value, t->bit);
                    label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                    if(ok)
                    {
                        Core::db().setLocalRamBitLabel(addr, t->value, t->bit, label);
                        refreshRom();
                    }
                }
                else
                {
                    QString label = Core::db().findGlobalRamBitLabelByAddr(t->value, t->bit);
                    bool ok = false;
                    auto title = QString().sprintf("Global RAM label at 0x%02x.%d", t->value, t->bit);
                    label = QInputDialog::getText(this, title, title, QLineEdit::Normal, label, &ok, Qt::Popup);
                    if(ok)
                    {
                        Core::db().setGlobalRamBitLabel(t->value, t->bit, label);
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
        QString comment = Core::db().findRomCommentByAddr(addr);
        bool ok = false;
        auto title = QString().sprintf("Set comment at 0x%04x", addr);
        comment = QInputDialog::getText(this, title, title, QLineEdit::Normal, comment, &ok, Qt::Popup);
        if(ok)
        {
            Core::db().setRomComment(addr, comment);
            refreshRom();
        }
    }
    else if(text == "x") // Cross references
    {
        QVector<QPair<uint16_t, QString>> xrefs;
        for(uint16_t xref : mBackend.lineInfo[selectedAddr()].referencedFrom)
        {
            auto tokens = mBackend.lineInfo[xref].toTokens(Core::db());
            QString text;
            for(auto & token : tokens)
                text.append(token.text);
            xrefs.append({xref, text});
        }
        if(!xrefs.empty())
        {
            mOldXrefsAddr = selectedAddr();
            mXrefsDialog->setXrefs(xrefs);
            mXrefsDialog->showNormal();
        }
    }
}

void DisassemblerTextEdit::mouseDoubleClickEvent(QMouseEvent*)
{
    auto t = selectedToken();
    if(t)
    {
        switch(t->type)
        {
        case Token::Rom:
            gotoAddress(t->value);
            break;
        default:
            qDebug() << "unimplemented double click token";
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
        mBackend.tokenLines[i] = mBackend.lineInfo[i].toTokens(Core::db());

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
