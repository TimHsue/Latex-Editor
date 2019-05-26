#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <Windows.h>
#include <QTextCursor>
#include "login.h"

namespace Ui {
    class MainWindow;
}

class PostThread;

class GV {
public:
    static DWORD lastUpdate;
    static DWORD threadTime;
    static std :: string cookie;
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

    void on_textbf_triggered();

    void on_emph_triggered();

    void on_underline_triggered();

    void on_login_triggered();

private:
    Ui::MainWindow *ui;
    PostThread *postThread;
    QTextCursor textCursor;
    login *loginClient;
};

#endif // MAINWINDOW_H
