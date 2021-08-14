#include "Headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <dirent.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <QFileDialog>

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

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_scanButton_clicked()
{
    ui->extensionsListWidget->clear();
    std::string root_path = ui->sourceLineEdit->text().toStdString();

    int file_count = 0;
    std::vector<std::string> extensions;
    for(std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(root_path))
    {   file_count++;
        std::string string_file_count = "Total File Count: " + std::to_string(file_count);
        ui->totalFilesLabel->setText(QString::fromStdString(string_file_count));

        std::string extenion = entry.path().extension();
        extensions.push_back(extenion);
    }

    std::unordered_set<std::string> unordered_set_extensions(extensions.begin(), extensions.end());
    extensions.clear();
    extensions.assign(unordered_set_extensions.begin(), unordered_set_extensions.end());
    std::sort(extensions.begin(), extensions.end());

    for(std::string s : extensions)
    {
        QListWidgetItem *new_item = new QListWidgetItem();
        new_item->setText(QString::fromStdString(s));
        ui->extensionsListWidget->addItem(new_item);
    }
    ui->sortButton->setEnabled(true);
}

void MainWindow::on_sortButton_clicked()
{
    ui->consolePlainTextEdit->clear();
    std::string root_path = ui->sourceLineEdit->text().toStdString();

    std::vector<std::string> selected_extensions;
    for(QListWidgetItem *item : ui->extensionsListWidget->selectedItems())
    {
        selected_extensions.push_back(item->text().toStdString());
    }

    int file_count = 0;
    int failed_count = 0;
    for(std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(root_path))
    {
        std::string file_ext = entry.path().extension();
        if(std::find(selected_extensions.begin(), selected_extensions.end(), file_ext) != selected_extensions.end())
        {
            std::string source_file = entry.path();
            std::string destination_file = ui->destinationLineEdit->text().toStdString();
            destination_file += entry.path().filename();

            try
            {
                std::filesystem::copy_file(source_file, destination_file);
                //std::cout << "Successful copy: " << destination_file << std::endl;
                std::string line = "Successful copy: " + destination_file;
                ui->consolePlainTextEdit->appendPlainText(QString::fromStdString(line));
                file_count++;
            }
            catch (std::filesystem::filesystem_error &e)
            {
                std::string line = e.code().message() + e.what();
                ui->consolePlainTextEdit->appendPlainText(QString::fromStdString(line));
                failed_count++;
            }

        }
        std::string string_file_count = "Total Files Copied: " + std::to_string(file_count);
        ui->totalFilesLabel->setText(QString::fromStdString(string_file_count));
        std::string string_failed_count = "Total Files Failed: " + std::to_string(failed_count);
        ui->failedCountLabel->setText(QString::fromStdString(string_failed_count));
    }
}

void MainWindow::on_sourceButton_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory("/home/");

    QString path = dialog.getExistingDirectory();
    path.append("/");
    ui->sourceLineEdit->setText(path);
}

void MainWindow::on_destinationButton_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory("/home/");

    QString path = dialog.getExistingDirectory();
    path.append("/");
    ui->destinationLineEdit->setText(path);
}
