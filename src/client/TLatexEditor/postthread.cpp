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
    isDownload = false;
    isUpload = false;
    pCookie = "";
    fileName = "";
    mutex.unlock();

    connect(this, SIGNAL(updateLatex(QString)),
            mainWindow_, SLOT(updateUI(QString)));
    connect(this, SIGNAL(saveLatex(QByteArray)),
            mainWindow_, SLOT(saveFileTo(QByteArray)));
    connect(this, SIGNAL(uploadResult(QString)),
            mainWindow_, SLOT(uploadResult(QString)));
}

void PostThread::updateUISlot(QString latexHTML) {}

void PostThread::run() {
    mutex.lock();
    if (isUpload) {
        isUpload = false;
        QByteArray transfer = content.toLatin1();
        mutex.unlock();
        QByteArray cookieBA = pCookie.toLatin1();
        QByteArray nameBA = fileName.toLatin1();
        Response response = postFile(ip, 8888, "uploadTex", transfer.data() ,
                                     false, cookieBA.data(), nameBA.data());

        return ;
    }

    if (isDownload) {
        std :: string requestType = isPdf ? "texToPdf" : "texToHtml";
        isDownload = false;
        QByteArray transfer = content.toLatin1();
        mutex.unlock();
        Response response = postFile(ip, 8888, requestType.c_str(), transfer.data() , false);
        QByteArray htmlBA;
        for (int i = 0; i < response.html.length(); i++) {
            htmlBA += (int)response.html[i];
        }
        emit saveLatex(htmlBA);
        return ;
    }
    mutex.unlock();

    while (true) {
        int nowTime = GetTickCount();
        if (nowTime - GV::lastUpdate > 4000) { // run after 4000ms
            GV::lastUpdate = nowTime;

            mutex.lock();
            if (needUpdate) {
                needUpdate = false;
                QByteArray transfer = content.toLatin1();
                mutex.unlock();
                Response response = postFile(ip, 8888, type, transfer.data() , false);
                emit updateLatex(QString::fromStdString(response.html));
            } else {
                mutex.unlock();
            }
        }
    }
    // ui->latexPreviwer->setHtml(QString::fromStdString(latexResult));
}
