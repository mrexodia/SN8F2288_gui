#include "XrefsDialog.h"
#include "ui_XrefsDialog.h"

XrefsDialog::XrefsDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::XrefsDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
}

XrefsDialog::~XrefsDialog()
{
    delete ui;
}

void XrefsDialog::setXrefs(const QVector<QPair<uint16_t, QString> > & xrefs)
{
    this->xrefs = xrefs;
    ui->listWidget->clear();
    for(auto && xref : xrefs)
        ui->listWidget->addItem(xref.second);
    ui->listWidget->setCurrentRow(0);
}

void XrefsDialog::on_listWidget_itemSelectionChanged()
{
    auto currentRow = ui->listWidget->currentRow();
    if(currentRow != -1)
        emit gotoAddress(xrefs[currentRow].first);
}
