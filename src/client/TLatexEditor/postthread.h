#ifndef POSTTHREAD_H
#define POSTTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "mainwindow.h"
#include "savefile.h"

class PostThread : public QThread {
    Q_OBJECT

private:
    char ip[16];
    char type[16];

signals:
    void updateLatex(QString latexHTML);

    void saveLatex(QByteArray content);

    void uploadResult(QString info);

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
    bool isDownload;
    bool isPdf;
    bool isUpload;
    QString content;
    QString pCookie;
    QString fileName;

};

#endif // POSTTHREAD_H
