#ifndef DISASSEMBLERTEXTEDIT_H
#define DISASSEMBLERTEXTEDIT_H

#include <QPlainTextEdit>
#include "DisassemblerHighlighter.h"
#include "DisassemblerBackend.h"
#include "XrefsDialog.h"

class DisassemblerTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit DisassemblerTextEdit(QWidget* parent = nullptr);
    DisassemblerHighlighter* highlighter() const { return mHighlighter; }
    DisassemblerBackend* backend() { return &mBackend; }
    std::vector<Token>* selectedLine() const;
    uint16_t selectedAddr() const;
    Token* selectedToken() const;

    bool loadCfg(const QString & file);
    bool saveCfg(const QString & file);
    void loadRom(const QString & file);
    void refreshRom();

public slots:
    bool gotoAddress(uint16_t addr);
    void xrefsRejectedSlot();

protected:
    void keyPressEvent(QKeyEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;

private:
    DisassemblerHighlighter* mHighlighter;

    DisassemblerBackend mBackend;
    XrefsDialog* mXrefsDialog;
    uint16_t mOldXrefsAddr = 0;
};

#endif // DISASSEMBLERTEXTEDIT_H
