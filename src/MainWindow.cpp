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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    maybeSave();
    Core::cpu()->stopCpu();
    Core::cpu()->wait();
    QMainWindow::closeEvent(e);
}

void MainWindow::on_action_Load_ROM_triggered()
{
    maybeSave();
    auto file = QFileDialog::getOpenFileName(this, "Load ROM", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if(file.isEmpty())
        return;
    file = QDir::toNativeSeparators(file);
    mRomPath = file;
    ui->disassembler->loadRom(file);
    updateTitle();
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

void MainWindow::on_actionRun_triggered()
{
    Core::cpu()->runCpu();
}

void MainWindow::on_actionPause_triggered()
{
    Core::cpu()->haltCpu();
}

void MainWindow::on_actionReset_triggered()
{
    Core::cpu()->haltCpu();
    Core::chip().reset(ResetPowerOn);
    Core::cpu()->paused(ChipCpu::RunPaused);
}
