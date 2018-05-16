#ifndef XREFSDIALOG_H
#define XREFSDIALOG_H

#include <QDialog>
#include "DisassemblerBackend.h"

namespace Ui {
class XrefsDialog;
}

class XrefsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit XrefsDialog(QWidget* parent = nullptr);
    ~XrefsDialog();
    void setXrefs(const QVector<QPair<uint16_t, QString>> & xrefs);

signals:
    void gotoAddress(uint16_t address);

private slots:
    void on_listWidget_itemSelectionChanged();

private:
    Ui::XrefsDialog *ui;
    QVector<QPair<uint16_t, QString>> xrefs;
};

#endif // XREFSDIALOG_H
