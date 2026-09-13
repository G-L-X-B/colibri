#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <memory>

#include <QtConcurrent/QtConcurrentRun>

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QIODevice>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QToolTip>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , config(nullptr)
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

void MainWindow::on_startButton_clicked()
{
    if (job.isRunning()) {
        stop_processing();
    } else {
        start_processing();
    }
}

void MainWindow::start_processing()
{
    gather_config();
    if (config != nullptr) {
        ui->startButton->setText(tr("Stop"));
        run_job();
    }
}

void MainWindow::gather_config()
{
    using std::cout;
    using std::endl;
    QString pattern = ui->fileMaskInput->text();
    QRegularExpression regex(QRegularExpression::anchoredPattern(pattern));
    if (!regex.isValid()) {
        QToolTip::showText(
            ui->fileMaskInput->pos() + pos(),
            tr("Invalid regular expression"));
        return;
    }

    QString inputPath = ui->inputPathInput->text();
    if (!QDir(inputPath).exists()) {
        QToolTip::showText(
            ui->inputPathInput->pos() + pos(),
            tr("An existing path is expected"));
        return;
    }

    QString outputPath = ui->outputPathInput->text();
    if (!QDir(outputPath).exists()) {
        QToolTip::showText(
            ui->outputPathInput->pos() + pos(),
            tr("An existing path is expected"));
        return;
    }

    if (!ui->bitMaskInput->hasAcceptableInput()) {
        QToolTip::showText(
            ui->bitMaskInput->pos() + pos(),
            tr("A full 8-byte mask is expected"));
        return;
    }

    std::shared_ptr<Config> tmp = std::make_shared<Config>();
    tmp->input_path = ui->inputPathInput->text().isEmpty()
                        ? ui->inputPathInput->placeholderText()
                        : ui->inputPathInput->text();
    tmp->output_path = ui->outputPathInput->text().isEmpty()
                        ? ui->outputPathInput->placeholderText()
                        : ui->outputPathInput->text();
    tmp->operator_ = ui->operatorComboBox->currentText();
    tmp->file_regex = regex;
    tmp->bitmask = parse_bit_mask(ui->bitMaskInput->text());
    tmp->policy = static_cast<Config::DuplicatesPolicy>(ui->duplicatePolicyGroup->checkedId());
    tmp->remove_processed = ui->deleteInputFilesCheckBox->isChecked();

    if (tmp->input_path == tmp->output_path) {
        QToolTip::showText(
            ui->outputPathInput->pos() + pos(),
            tr("Output path must differ from input path"));
        return;
    }

    cout << "input_path: " << tmp->input_path.toStdString() << endl;
    cout << "output_path: " << tmp->output_path.toStdString() << endl;
    cout << "operator_: " << tmp->operator_.toStdString() << endl;
    cout << "file_regex: " << tmp->file_regex.pattern().toStdString() << endl;
    cout << "bitmask: " << std::hex << tmp->bitmask << std::dec << endl;
    cout << "policy: " << tmp->policy << endl;
    cout << "remove_processed: " << std::boolalpha << tmp->remove_processed << endl;

    config.swap(tmp);
}

void MainWindow::run_job()
{
    if (job.isRunning())
        return;
    job = QtConcurrent::run(&MainWindow::process_files, this);
}

void MainWindow::process_files(QPromise<void> &promise)
{
    using std::cout;
    using std::endl;
    cout << "<< process_files" << endl;

    QDir source_dir(config->input_path);
    QStringList file_list = source_dir.entryList(QDir::Files);
    file_list = filter_matching_filenames(file_list);
    bool stop = false;

    for (qsizetype i = 0; i < file_list.size() && !stop; ++i) {
        cout << "iteration: " << i << endl;
        cout << "file name: " << file_list[i].toStdString() << endl;

        process_file(promise, file_list[i]);

        promise.suspendIfRequested();
        stop = promise.isCanceled();
    }
    finish_job();
    cout << ">> process_files" << endl;
}

void MainWindow::finish_job()
{
    clean_up();
}

void MainWindow::clean_up()
{
    ui->startButton->setText(tr("Start"));
    config.reset();
}

void MainWindow::stop_processing()
{
    if (job.isRunning()) {
        job.cancel();
        job.waitForFinished();
    }
}

void MainWindow::process_file(QPromise<void> &promise, const QString &file_name)
{
    using std::cout;
    using std::endl;

    uint64_t mask = config->bitmask;
    QDir source_dir(config->input_path);
    QDir dest_dir(config->output_path);

    QFile in_file(source_dir.filePath(file_name));
    in_file.open(QIODevice::ReadOnly);
    QDataStream in(&in_file);

    QString dest_file_name;
    if (config->policy == Config::DuplicatesPolicy::kRename)
        dest_file_name = find_new_file_name(dest_dir, file_name);
    else
        dest_file_name = file_name;
    cout << "dest_file name: " << dest_file_name.toStdString() << endl;

    QFile dest_file(dest_dir.filePath(dest_file_name));
    dest_file.open(QIODevice::WriteOnly);
    QDataStream out(&dest_file);

    bool stop = false;

    uint64_t buffer;
    qint64 len;
    while (!in.atEnd() && !stop) {
        len = in.readRawData((char *)&buffer, 8);
        if (config->operator_ == "AND") {
            buffer &= mask;
        } else if (config->operator_ == "OR") {
            buffer |= mask;
        } else if (config->operator_ == "XOR") {
            buffer ^= mask;
        }
        out.writeRawData((char *)&buffer, len);

        promise.suspendIfRequested();
        stop = promise.isCanceled();
    }
    in_file.close();
    dest_file.close();
    if (!stop && config->remove_processed)
        in_file.remove();
}

uint64_t MainWindow::parse_bit_mask(const QString &source)
{
    QStringList bytes = source.split(" ", Qt::SkipEmptyParts);
    uint64_t mask = 0ULL;
    for (qsizetype i = 0; i < bytes.size(); ++i) {
        uint64_t b = bytes[i].toUShort(nullptr, 16);
        uint power = bytes.size() * (7 - i);
        mask |= b << power;
    }
    return mask;
}

QStringList MainWindow::filter_matching_filenames(const QStringList &source)
{
    QStringList res;
    for (const QString &name : source) {
        QRegularExpressionMatch match = config->file_regex.match(name);
        if (match.hasMatch()) {
            res.push_back(name);
        }
    }
    return res;
}

QString MainWindow::find_new_file_name(const QDir &dir, const QString &name)
{
    QFile file(dir.filePath(name));
    int next_index = 1;
    while (file.exists()) {
        file.setFileName(dir.filePath(name + "-" + QString::number(next_index)));
        ++next_index;
    }
    return file.fileName();
}



























