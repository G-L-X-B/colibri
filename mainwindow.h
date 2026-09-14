#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QFuture>
#include <QList>
#include <QMainWindow>
#include <QPromise>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTimer>


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
        struct ProgressData {
            QString input_filename;
            QString output_filename;
            qsizetype file_size;
            qsizetype progress;
        };

        QString input_path;
        QString output_path;
        QString operator_;
        QRegularExpression file_regex;
        uint64_t bitmask;
        DuplicatesPolicy policy;
        bool remove_processed;

        bool started;

        QList<ProgressData> progress;
    };

    void start_processing();

    void gather_config();

    void run_job();

    void process_files(QPromise<void> &promise);

    void prepare_files();

    void process_file(QPromise<void> &promise, Config::ProgressData &file_name);

    void finish_job();
    void clean_up();

    // stop, then resume
    void stop_processing();

    void resume_processing();

    // abort, drop progress
    void cancel_processing();

    void warn_bad_timing();

    uint64_t parse_bit_mask(const QString &source);
    QStringList filter_matching_filenames(const QStringList &source);
    QString find_new_file_name(const QDir &dir, const QString &name);

    QFuture<void> job;
    Ui::MainWindow *ui;
    QSharedPointer<QTimer> timer;
    QSharedPointer<Config> config;
};

#endif // MAINWINDOW_H
