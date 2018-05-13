#ifndef DISASSEMBLERTEXTEDIT_H
#define DISASSEMBLERTEXTEDIT_H

#include <QPlainTextEdit>
#include "DisassemblerHighlighter.h"
#include "DisassemblerBackend.h"

class DisassemblerTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit DisassemblerTextEdit(QWidget* parent = nullptr);
    DisassemblerHighlighter* highlighter() const { return mHighlighter; }
    std::vector<Token>* selectedLine() const;
    uint16_t selectedAddr() const;
    Token* selectedToken() const;

    void loadCfg(const QString & file);
    void loadRom(const QString & file);
    void refreshRom();

protected:
    void keyPressEvent(QKeyEvent*) override;

private:
    DisassemblerHighlighter* mHighlighter;
    QColor disassemblerBackground = "#FFF8F0";
    DisassemblerBackend mBackend;
};

#endif // DISASSEMBLERTEXTEDIT_H
