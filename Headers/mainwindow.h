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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

    void on_scanButton_clicked();

    void on_sortButton_clicked();

    void on_sourceButton_clicked();

    void on_destinationButton_clicked();

    void on_selectButton_clicked();

    void on_actionAbout_GnuSort_triggered();

    void clear_ui(bool clear_extensions);

private:
    Ui::MainWindow *ui;

    int total_files;

};

#endif // MAINWINDOW_H
