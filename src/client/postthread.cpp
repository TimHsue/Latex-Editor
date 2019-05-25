#include "post.cpp"
#include "postthread.h"
#include "log.cpp"

PostThread::~PostThread() { }

PostThread::PostThread(MainWindow *mainWindow_) {
    mainWindow = mainWindow_;
    strcpy_s(ip, "106.52.251.85");
    strcpy_s(type, "texToHtml");

    mutex.lock();
    running = false;
    mutex.unlock();

    connect(this, SIGNAL(updateLatex(QString, QString)),
            mainWindow_, SLOT(updateUI(QString, QString)));
}

void PostThread::updateUISlot(QString latexHTML) {}

void PostThread::run() {
    while (true) {
        DWORD nowTime = GetTickCount();
        if (nowTime - GV::lastUpdate > 1500) { // run after 1500ms
            GV::lastUpdate = nowTime;

            mutex.lock();
            if (needUpdate) {
                needUpdate = false;
                QByteArray transfer = content.toLatin1();
                mutex.unlock();
                Response response = postFile(ip, 8888, type, transfer.data() , false);
                emit updateLatex(QString::fromStdString(response.html),
                                 QString::fromStdString(response.css));
            } else {
                mutex.unlock();
            }
        }
    }
    // ui->latexPreviwer->setHtml(QString::fromStdString(latexResult));
}
