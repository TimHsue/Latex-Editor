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

    connect(this, SIGNAL(updateLatex(QString)), mainWindow_, SLOT(updateUI(QString)));
}

void PostThread::updateUISlot(QString latexHTML) {
    mainWindow -> updateUI(latexHTML);
}

void PostThread::run() {
    while (true) {
        DWORD nowTime = GetTickCount();
        if (nowTime - GV::lastUpdate > 1500) { // terminate after 4000ms
            GV::lastUpdate = nowTime;

            mutex.lock();
            if (needUpdate) {
                needUpdate = false;
                QByteArray transfer = content.toLatin1();
                mutex.unlock();
                std :: string latexResult = postFile(ip, 8888, type, transfer.data() , false);
                emit updateLatex(QString::fromStdString(latexResult));
            }
        }
    }
    // ui->latexPreviwer->setHtml(QString::fromStdString(latexResult));
}
