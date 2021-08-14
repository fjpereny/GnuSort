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

private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
