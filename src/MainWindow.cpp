#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <queue>
#include <unordered_set>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>

#include "RangesDialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mBaseTitle = windowTitle();
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
    mRomPath = "/Users/duncan/Projects/SN8F2288_disassembler/cmake-build-debug/lenovo.rom";
    ui->disassembler->loadRom(mRomPath);
    updateTitle();
    //ui->disassembler->loadRom("/Users/duncan/Library/Application Support/Binary Ninja/plugins/sn8f2288_binja/binja.rom");
}

void MainWindow::on_actionRefresh_triggered()
{
    ui->disassembler->refreshRom();
}

void MainWindow::on_actionImport_dissn8_cfg_triggered()
{
    auto file = QFileDialog::getOpenFileName(this, "Load Database", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "Databases (*.cfg)");
    if(file.isEmpty())
        return;
    file = QDir::toNativeSeparators(file);
    if(ui->disassembler->loadCfg(file))
    {
        mDbPath = file;
        updateTitle();
    }
}

void MainWindow::on_actionROM_Ranges_triggered()
{
    RangesDialog ranges(&ui->disassembler->backend()->db, this);
    if(ranges.exec() != QDialog::Accepted)
        return;
}

void MainWindow::on_actionSave_Database_triggered()
{
    if(mDbPath.isEmpty())
        on_actionSave_Database_as_triggered();
    else
        ui->disassembler->saveCfg(mDbPath);
}

void MainWindow::on_actionSave_Database_as_triggered()
{
    auto file = QFileDialog::getSaveFileName(this, "Save Database", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "Databases (*.cfg)");
    if(file.isEmpty())
        return;
    file = QDir::toNativeSeparators(file);
    if(ui->disassembler->saveCfg(file))
    {
        mDbPath = file;
        updateTitle();
    }
}

void MainWindow::updateTitle()
{
    auto getFilename = [](const QString & file)
    {
        QFileInfo fi(file);
        return fi.fileName();
    };
    auto newTitle = mBaseTitle;
    if(!mRomPath.isEmpty())
        newTitle += QString(" - %1").arg(getFilename(mRomPath));
    if(!mDbPath.isEmpty())
        newTitle += QString(" [%1]").arg(getFilename(mDbPath));
    setWindowTitle(newTitle);
}
