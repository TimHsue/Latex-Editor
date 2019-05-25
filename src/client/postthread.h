#ifndef POSTTHREAD_H
#define POSTTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "mainwindow.h"

class PostThread : public QThread {
    Q_OBJECT

private:
    char ip[16];
    char type[16];

signals:
    void updateLatex(QString latexHTML);

public slots:
    void updateUISlot(QString latexHTML);

public:
    ~PostThread();
    PostThread(MainWindow* mainWindow_);
    void run();
    MainWindow *mainWindow;
    QMutex mutex;
    bool running;
    bool needUpdate;
    QString content;
};

#endif // POSTTHREAD_H
