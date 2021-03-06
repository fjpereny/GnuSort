#include "Headers/mainwindow.h"
#include "Headers/aboutdialog.h"
#include "ui_mainwindow.h"
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cmath>



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


void MainWindow::clear_ui(bool clear_extensions=true)
{
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(false);

    if (clear_extensions)
        ui->extensionsListWidget->clear();

    ui->totalFilesLabel->setText("");
    ui->fileSizeLabel->setText("");
    ui->copiedFilesLabel->setText("");
    ui->failedCountLabel->setText("");
    ui->remainingFilesLabel->setText("");

    total_files = 0;
}


struct File_Size
{
    uint iTB = 0;
    uint iGB = 0;
    uint iMB = 0;
    uint iKB = 0;
    uint64_t iB = 0;

    // Conversions from Byte
    const uint64_t  TB =    1000000000000;
    const uint      GB =    1000000000;
    const uint      MB =    1000000;
    const uint      KB =    1000;
    const uint      B =     1;


    void push_up_units()
    {
        while (iB >= 1000)
        {
            iKB += 1;
            iB -= 1000;
        }
        while (iKB >= 1000)
        {
            iMB += 1;
            iKB -= 1000;
        }
        while (iMB >= 1000)
        {
            iGB += 1;
            iMB -= 1000;
        }
        while (iGB >= 1000)
        {
            iTB += 1;
            iMB -= 1000;
        }
    }


    void add_size(uint bytes)
    {
        iB += bytes;
        push_up_units();
    }


    std::string get_unit()
    {
        if (iTB >= 1)
            return "TB";
        else if (iGB >= 1)
            return "GB";
        else if (iMB >= 1)
            return "MB";
        else if (iKB >= 1)
            return "KB";
        else
            return "B";
    }


    double size_value()
    {
        double size = 0;

        if (iTB >= 1)
        {
            size += iTB;
            size += (float)iGB / 1000.0;
            size += (float)iMB / 1000000.0;
            size += (float)iKB / 1000000000.0;
            size += (float)iB  / 1000000000000.0;
        }

        if (iGB >= 1)
        {
            size += iGB;
            size += (float)iMB / 1000.0;
            size += (float)iKB / 1000000.0;
            size += (float)iB  / 1000000000.0;
        }
        else if (iMB >= 1)
        {
            size += iMB;
            size += (float)iKB / 1000.0;
            size += (float)iB  / 1000000.0;
        }
        else if (iKB >= 1)
        {
            size += iKB;
            size += (float)iB / 1000.0;
        }
        else
            size += iB;

        return size;
    }


    QString print_size()
    {
       return "File Size: " + QString::number(size_value(), 'f', 2) + QString::fromStdString(get_unit());
    }

    void clear()
    {
        iGB = 0;
        iMB = 0;
        iKB = 0;
        iB = 0;
    }
};


void MainWindow::on_scanButton_clicked()
{
    // GUI Cleanup
    clear_ui();

    File_Size file_size;

    std::string root_path = ui->sourceLineEdit->text().toStdString();


    if (!std::filesystem::directory_entry(root_path).exists())
    {
        QMessageBox *msg_box = new QMessageBox(this);
        msg_box->setText("The source directory does not exist.");
        msg_box->show();
        return;
    }

    std::vector<std::string> extensions;
    try
    {
        int processEvent_counter = 0;
        int processEvent_interval = 500;

        QString string_file_size = QString::fromStdString("Total File Size: 0KB");
        ui->scanFileSizeLabel->setText(string_file_size);



        for(std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(root_path))
        {
            processEvent_counter++;

            if ((!entry.is_directory()) && entry.is_regular_file())
            {
                file_size.add_size(entry.file_size());
                total_files++;
            }


            QString string_file_count = "Total File Count: " + QString::number(total_files);
            ui->scanTotalFilesLabel->setText(string_file_count);

            ui->scanFileSizeLabel->setText(file_size.print_size());

            std::string extenion = entry.path().extension();
            extensions.push_back(extenion);

            if (processEvent_counter % processEvent_interval == 0)
                QCoreApplication::processEvents();
        }

        std::unordered_set<std::string> unordered_set_extensions(extensions.begin(), extensions.end());
        extensions.clear();
        extensions.assign(unordered_set_extensions.begin(), unordered_set_extensions.end());
        std::sort(extensions.begin(), extensions.end());


        for (std::string s : extensions)
        {
            QListWidgetItem *new_item = new QListWidgetItem(ui->extensionsListWidget);
            new_item->setText(QString::fromStdString(s));
            ui->extensionsListWidget->addItem(new_item);
        }

        ui->totalExtensionsLabel->setText("Total Extensions: " + QString::number(extensions.size()));

        ui->selectButton->setEnabled(true);
    }

    catch (std::filesystem::filesystem_error &e)
    {
        return;
    }




}



void MainWindow::on_selectButton_clicked()
{
    clear_ui(false);

    std::string root_path = ui->sourceLineEdit->text().toStdString();

    total_files = 0;

    File_Size file_size;

    std::vector<std::string> selected_extensions;
    for(QListWidgetItem *item : ui->extensionsListWidget->selectedItems())
    {
                selected_extensions.push_back(item->text().toStdString());
    }

    try
    {
        for(std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(root_path))
        {
            if ((!entry.is_directory()) && entry.is_regular_file())
            {
                if(std::find(selected_extensions.begin(), selected_extensions.end(), entry.path().extension()) != selected_extensions.end())
                    {
                        file_size.add_size(entry.file_size());
                        total_files++;
                    }

                QString string_file_count = "Total File Count: " + QString::number(total_files);
                ui->totalFilesLabel->setText(string_file_count);

                ui->fileSizeLabel->setText(file_size.print_size());
            }
        }
        ui->sortButton->setEnabled(true);
    }
    catch (std::filesystem::filesystem_error &e)
    {
        return;
    }
}


void MainWindow::on_sortButton_clicked()
{
    ui->selectButton->setEnabled(false);
    ui->sortButton->setEnabled(false);


    ui->progressBar->show();
    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(total_files);


    ui->copiedFilesLabel->setText("Copied Files: 0");
    ui->failedCountLabel->setText("Copy Failures: 0");


    std::string destination_path = ui->destinationLineEdit->text().toStdString();

    #ifdef linux
    if (destination_path.back() != '/')
        destination_path += '/';
    #endif

    #ifdef _WIN32
    if (destination_path.back() != '\\')
        destination_path += '\\';
    #endif


    if (!std::filesystem::directory_entry(destination_path).exists())
    {
        std::filesystem::create_directory(destination_path);

        // If folder doesn't exist after attempting to create it, exit the sort function.
        if (!std::filesystem::directory_entry(destination_path).exists())
        {
            return;
        }
    }


    if (ui->extensionFolderCheckbox->isChecked())
    {
        for (QListWidgetItem *extension : ui->extensionsListWidget->selectedItems())
        {
            std::string folder_name;
            if (extension->text() != "")
                folder_name = extension->text().toStdString().erase(0, 1);
            else
                folder_name = "No Extension";

            std::string full_folder_path = destination_path + folder_name;
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
        if(!std::filesystem::exists("logs\\"))
        {
            std::filesystem::create_directory("logs\\");
        }
        #endif



        if (ui->logCheckBox->isChecked())
        {
            std::time_t t = std::time(nullptr);
            std::string cur_time = std::asctime(std::localtime(&t));
            std::string log_file_name = cur_time + "log.txt";

            std::string log_path = ui->logFolderLineEdit->text().toStdString();

            #ifdef linux
            if (*log_path.end() != '/')
                log_path += '/';
            #endif

            #ifdef _WIN32
            if (*log_path.end() != '\\')
                log_path += '\\';
            #endif

            log_file.open(log_path + log_file_name);
        }


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
            std::string destination_file = destination_path;

            if(!entry.is_directory())
            {
                if (ui->extensionFolderCheckbox->isChecked())
                {
                    destination_file += file_ext.erase(0, 1);
                    #ifdef linux
                    destination_file += "/";
                    #endif
                    #ifdef _WIN32
                    destination_file += "\\";
                    #endif
                }


                if (entry.path().extension() == "")
                {
                    #ifdef  linux
                    destination_file += "/No Extension/";
                    #endif

                    #ifdef _WIN32
                    destination_file += "\\No Extension\\";
                    #endif
                }

                destination_file += entry.path().filename();


                try
                {
                    if (ui->keepOriginalsCheckbox->isChecked())
                        std::filesystem::copy_file(source_file, destination_file);
                    else
                        std::filesystem::rename(source_file, destination_file);
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
    dialog.setDirectory("C:\\");
#endif

    QString path = dialog.getExistingDirectory();

#ifdef linux
    if (path != "/")
        path.append("/");
#endif

#ifdef _WIN32
    path.append("\\");
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
    dialog.setDirectory("C:\\");
#endif

    QString path = dialog.getExistingDirectory();
#ifdef linux
    if (path != "/")
        path.append("/");
#endif

#ifdef _WIN32
    path.append("\\");
#endif
    ui->destinationLineEdit->setText(path);
}


void MainWindow::on_actionAbout_GnuSort_triggered()
{
    AboutDialog *about = new AboutDialog();
    about->show();
}
