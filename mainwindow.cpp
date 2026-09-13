#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>


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
