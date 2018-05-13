#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <queue>
#include <unordered_set>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Load_ROM_triggered()
{
    /*auto file = QFileDialog::getOpenFileName(this, "Load ROM", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if(file.isEmpty())
        return;
    file = QDir::toNativeSeparators(file);
    ui->disassembler->loadRom(file);*/
    ui->disassembler->loadRom("/Users/duncan/Projects/SN8F2288_disassembler/cmake-build-debug/lenovo.rom");
    //ui->disassembler->loadRom("/Users/duncan/Library/Application Support/Binary Ninja/plugins/sn8f2288_binja/binja.rom");
}

void MainWindow::on_actionRefresh_triggered()
{
    ui->disassembler->refreshRom();
}

void MainWindow::on_actionImport_dissn8_cfg_triggered()
{
    auto file = QFileDialog::getOpenFileName(this, "Load CFG", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if(file.isEmpty())
        return;
    file = QDir::toNativeSeparators(file);
    ui->disassembler->loadCfg(file);
}
