#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <queue>
#include <unordered_set>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include "Config.h"
#include "RangesDialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mBaseTitle = windowTitle();
    ui->splitter->setStretchFactor(0, 70);
    ui->splitter->setStretchFactor(1, 30);

    QPalette p;
    p.setColor(QPalette::Base, Config().disassemblerBackground);

    setPalette(p);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    maybeSave();
    QMainWindow::closeEvent(e);
}

void MainWindow::on_action_Load_ROM_triggered()
{
    /*auto file = QFileDialog::getOpenFileName(this, "Load ROM", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if(file.isEmpty())
        return;
    file = QDir::toNativeSeparators(file);
    ui->disassembler->loadRom(file);*/
    maybeSave();
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
    RangesDialog ranges(&Core::db(), this);
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

void MainWindow::on_actionStep_triggered()
{
    Core::cpu()->stepCpu();
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

void MainWindow::maybeSave()
{
    if(Core::db().unsavedChanges())
    {
        if(QMessageBox::question(this, "Save database?", "There are unsaved changes in the database, do you want to save?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
            on_actionSave_Database_triggered();
    }
}
