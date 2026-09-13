#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

    Ui::MainWindow *ui;
    Config *config;
    bool running;
};

#endif // MAINWINDOW_H
