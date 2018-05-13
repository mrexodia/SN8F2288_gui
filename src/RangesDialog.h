#ifndef RANGESDIALOG_H
#define RANGESDIALOG_H

#include <QDialog>
#include "Database.h"

namespace Ui {
class RangesDialog;
}

class RangesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RangesDialog(Database* db, QWidget *parent = 0);
    ~RangesDialog();

private slots:
    void on_listWidget_customContextMenuRequested(const QPoint &pos);

private:
    Ui::RangesDialog *ui;
    Database* db;
};

#endif // RANGESDIALOG_H
