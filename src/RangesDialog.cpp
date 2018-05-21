#include "RangesDialog.h"
#include "ui_RangesDialog.h"
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

RangesDialog::RangesDialog(Database* db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RangesDialog),
    db(db)
{
    ui->setupUi(this);
    setModal(true);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    for(auto & range : db->getRomRanges())
        ui->listWidget->addItem(QString().sprintf("0x%04x-0x%04x", range.first, range.second));
}

RangesDialog::~RangesDialog()
{
    delete ui;
}

static bool parseRange(const QString & text, QPair<uint16_t, uint16_t> & range)
{
    QStringList split = text.split('-');
    if(split.length() == 2)
    {
        bool ok1 = false, ok2 = false;
        range.first = split[0].toUShort(&ok1, 16);
        range.second = split[1].toUShort(&ok2, 16);
        return ok1 && ok2;
    }
    return false;
}

void RangesDialog::on_listWidget_customContextMenuRequested(const QPoint & pos)
{
    QMenu menu(this);
    menu.addAction("Add", [&]()
    {
        bool ok = false;
        QString text = QInputDialog::getText(this, "Enter range", "0xXXXX-0xYYYY", QLineEdit::Normal, QString(), &ok);
        if(ok)
        {
            QPair<uint16_t, uint16_t> range;
            if(parseRange(text, range) && range.first < range.second && range.first < 0x3000 && range.second < 0x3000 && !db->findRomRangeConst(range.first) && !db->findRomRangeConst(range.second))
            {
                db->addRomRange(range.first, range.second);
                ui->listWidget->addItem(QString().sprintf("0x%04x-0x%04x", range.first, range.second));
            }
            else
                QMessageBox::warning(this, "Error", QString("Invalid range '%1'").arg(text));
        }
    });
    if(!ui->listWidget->selectedItems().isEmpty())
    {
        menu.addAction("Remove", [&]()
        {
            for(auto item : ui->listWidget->selectedItems())
            {
                QPair<uint16_t, uint16_t> range;
                parseRange(item->text(), range);
                auto found = db->findRomRangeConst(range.first);
                if(found)
                {
                    auto count = found->ramBitLabels.count() + found->ramLabels.count();
                    if(!count || QMessageBox::question(this, "Confirmation", QString("Are you sure you want to delete this range and %1 labels?").arg(count), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                    {
                        db->deleteRomRange(range.first);
                        delete item;
                    }
                }
                else
                    QMessageBox::warning(this, "Error", "This should never happen...");
            }
        });
    }
    menu.exec(ui->listWidget->mapToGlobal(pos));
}
