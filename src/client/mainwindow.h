#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <Windows.h>

namespace Ui {
    class MainWindow;
}

class PostThread;

class GV {
public:
    static DWORD lastUpdate;
    static DWORD threadTime;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateUI(QString latexHTML, QString latexCSS);

private slots:
    void on_latexEditor_textChanged();

private:
    Ui::MainWindow *ui;
    PostThread *postThread;
};

#endif // MAINWINDOW_H
