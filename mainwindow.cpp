#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , config(nullptr)
    , running(false)
{
    ui->setupUi(this);

    ui->inputPathInput->setPlaceholderText(QDir::currentPath());
    ui->outputPathInput->setPlaceholderText(QDir::currentPath());

    ui->duplicatePolicyGroup->setId(
        ui->renameButton,
        Config::DuplicatesPolicy::kRename);
    ui->duplicatePolicyGroup->setId(
        ui->rewriteButton,
        Config::DuplicatesPolicy::kRewrte);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_inputPathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(
        nullptr,
        tr("Select an input directory"),
        ui->inputPathInput->text()
    );
    ui->inputPathInput->setText(path);
}


void MainWindow::on_outputPathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(
        nullptr,
        tr("Select an output directory"),
        ui->outputPathInput->text()
        );
    ui->outputPathInput->setText(path);
}


