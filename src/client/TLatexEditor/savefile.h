#ifndef SAVEFILE_H
#define SAVEFILE_H

#include <QMainWindow>
#include "mainwindow.h"

namespace Ui {
    class SaveFile;
}

class PostThread;

class SaveFile : public QMainWindow {
    Q_OBJECT

public:
    explicit SaveFile(QWidget *parent = nullptr, MainWindow *mainWindow_ = nullptr);
    ~SaveFile();

signals:
    void saveFileGot(QByteArray content);

private slots:
    void on_showDirector_clicked();

    void on_saveButton_clicked();

private:
    Ui::SaveFile *ui;
    MainWindow *mainWindow;
};

#endif // SAVEFILE_H
