#include "Headers/mainwindow.h"
#include "Headers/aboutdialog.h"
#include "ui_mainwindow.h"
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <QFileDialog>
#include <iostream>
#include <fstream>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    total_files = 0;
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
    ui->progressBar->setVisible(false);
    ui->extensionsListWidget->clear();
    ui->copiedFilesLabel->setText("Copied Files: ");
    ui->failedCountLabel->setText("Copy Failures: ");
    ui->remainingFilesLabel->setText("Remaining Files: ");

    std::string root_path = ui->sourceLineEdit->text().toStdString();

    int file_count = 0;
    total_files = 0;
    std::vector<std::string> extensions;
    try
    {
        for(std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(root_path))
        {
            if (!entry.is_directory())
            {
                file_count++;
                total_files++;
            }
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
        ui->selectButton->setEnabled(true);
    }
    catch (std::filesystem::filesystem_error &e)
    {
        return;
    }

}

void MainWindow::on_sortButton_clicked()
{
    ui->progressBar->show();
    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(total_files);
    ui->copiedFilesLabel->setText("Copied Files: ");
    ui->failedCountLabel->setText("Copy Failures: ");


    if (ui->extensionFolderCheckbox->isChecked())
    {
        std::string path = ui->destinationLineEdit->text().toStdString();
        for (QListWidgetItem *extension : ui->extensionsListWidget->selectedItems())
        {
            std::string folder_name = extension->text().toStdString().erase(0, 1);
            std::string full_folder_path = path + folder_name;
            if (!std::filesystem::exists(full_folder_path))
            {
                std::filesystem::create_directory(full_folder_path);
            }
        }
    }


    std::ofstream log_file;
    if (ui->logCheckBox->isChecked())
    {
    #ifdef linux
        if(!std::filesystem::exists("logs/"))
        {
            std::filesystem::create_directory("logs");
        }
    #endif

    #ifdef _WIN32
        if(!std::filesystem::exists("logs\"))
        {
            std::filesystem::create_directory("logs\");
        }
    #endif

    std::time_t t = std::time(nullptr);
    std::string cur_time = std::asctime(std::localtime(&t));
    std::string log_file_name = cur_time + "log.txt";

#ifdef linux
    log_file.open("logs/" + log_file_name);
#endif

#ifdef _WIN32
    log_file.open("logs\" + log_file_name);
#endif             
    }

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

            if(!entry.is_directory())
            {
                if (ui->extensionFolderCheckbox->isChecked())
                {
                destination_file += file_ext.erase(0, 1);
                #ifdef linux
                destination_file += "/";
                #endif
                #ifdef _WIN32
                destination_file += "\";
                #endif
                }
                destination_file += entry.path().filename();

                try
                {
                    std::filesystem::copy_file(source_file, destination_file);
                    file_count++;

                      std::string line = "Successful copy: " + destination_file;
                    if (ui->logCheckBox->isChecked())
                      log_file << line << std::endl;
                }
                catch (std::filesystem::filesystem_error &e)
                {
                    std::string line = e.code().message() + e.what();
                    failed_count++;

                    if (ui->logCheckBox->isCheckable())
                        log_file << line << std::endl;
                }

            }
    }

        std::string string_file_count = "Total Files Copied: " + std::to_string(file_count);
        ui->copiedFilesLabel->setText(QString::fromStdString(string_file_count));
        std::string string_failed_count = "Total Files Failed: " + std::to_string(failed_count);
        ui->failedCountLabel->setText(QString::fromStdString(string_failed_count));

        int remaining_files = total_files - file_count - failed_count;
        std::string remaining_count = "Remaining Files: " + std::to_string(remaining_files);
        ui->remainingFilesLabel->setText(QString::fromStdString(remaining_count));

        int progress;
        try
        {
            progress = (file_count + failed_count);
        }
        catch (std::overflow_error &e)
        {
                  return;
        }
        ui->progressBar->setValue(progress);
        QCoreApplication::processEvents();
    }

    if (ui->logCheckBox->isChecked())
        log_file.close();
}

void MainWindow::on_sourceButton_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);

#ifdef linux
    dialog.setDirectory("/home/");
#endif

#ifdef _WIN32
    dialog.setDirectory("C:\");
#endif

    QString path = dialog.getExistingDirectory();

#ifdef linux
    if (path != "/")
        path.append("/");
#endif

#ifdef _WIN32
    path.append("\");
#endif
    ui->sourceLineEdit->setText(path);
}

void MainWindow::on_destinationButton_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
#ifdef linux
    dialog.setDirectory("/home/");
#endif

#ifdef _WIN32
    dialog.setDirectory("C:\");
#endif

    QString path = dialog.getExistingDirectory();
#ifdef linux
    if (path != "/")
        path.append("/");
#endif

#ifdef _WIN32
    path.append("\");
#endif
    ui->destinationLineEdit->setText(path);
}

void MainWindow::on_selectButton_clicked()
{
    ui->copiedFilesLabel->setText("Copied Files: ");
    ui->failedCountLabel->setText("Copy Failures: ");
    ui->remainingFilesLabel->setText("Remaining Files: ");

    std::string root_path = ui->sourceLineEdit->text().toStdString();

    total_files = 0;
    std::vector<std::string> selected_extensions;
    for(QListWidgetItem *item : ui->extensionsListWidget->selectedItems())
    {
                selected_extensions.push_back(item->text().toStdString());
    }

    try
    {
        for(std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(root_path))
        {
            if (!entry.is_directory())
            {
                if(std::find(selected_extensions.begin(), selected_extensions.end(), entry.path().extension()) != selected_extensions.end())
                    {
                        total_files++;
                    }
                std::string string_file_count = "Total File Count: " + std::to_string(total_files);
                ui->totalFilesLabel->setText(QString::fromStdString(string_file_count));
            }
        }
        ui->sortButton->setEnabled(true);
    }
    catch (std::filesystem::filesystem_error &e)
    {
        return;
    }
}


void MainWindow::on_actionAbout_GnuSort_triggered()
{
    AboutDialog *about = new AboutDialog();
    about->show();
}

