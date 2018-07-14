#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent*) override;

private slots:
    void on_action_Load_ROM_triggered();
    void on_actionRefresh_triggered();
    void on_actionImport_dissn8_cfg_triggered();
    void on_actionROM_Ranges_triggered();
    void on_actionSave_Database_triggered();
    void on_actionSave_Database_as_triggered();
    void on_actionStep_triggered();
    void on_actionRun_triggered();
    void on_actionPause_triggered();
    void on_actionReset_triggered();

private:
    Ui::MainWindow *ui;
    QString mRomPath;
    QString mDbPath;
    QString mBaseTitle;

    void updateTitle();
    void maybeSave();
};

#endif // MAINWINDOW_H
