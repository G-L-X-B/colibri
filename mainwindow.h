#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QFuture>
#include <QMainWindow>
#include <QPromise>
#include <QRegularExpression>
#include <QString>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_inputPathButton_clicked();

    void on_outputPathButton_clicked();

    void on_startButton_clicked();

private:
    struct Config {
        enum DuplicatesPolicy {kRewrte, kRename};

        QString input_path;
        QString output_path;
        QString operator_;
        QRegularExpression file_regex;
        uint64_t bitmask;
        DuplicatesPolicy policy;
        bool remove_processed;
    };

    void start_processing();

    void gather_config();

    void run_job();

    void process_files(QPromise<void> &promise);


    uint64_t parse_bit_mask(const QString &source);

    QFuture<void> job;
    Ui::MainWindow *ui;
    std::shared_ptr<Config> config;
};

#endif // MAINWINDOW_H
