#include "RegisterTable.h"
#include "Config.h"

#include <QHeaderView>

RegisterTable::RegisterTable(QWidget* parent)
    : QTableWidget(parent)
{
    QPalette p;
    p.setColor(QPalette::Base, Config().disassemblerBackground);
    setPalette(p);
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);

    setRowCount(1);
    setColumnCount(1);
    this->setItem(0, 0, new QTableWidgetItem("Hello world"));
}
