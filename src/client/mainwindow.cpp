#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "postthread.h"
#include "log.cpp"

DWORD GV::lastUpdate = 0;
DWORD GV::threadTime = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    postThread = new PostThread(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateUI(QString latexHTML) {
    ui->latexPreviwer->setHtml(latexHTML);
}

void MainWindow::on_latexEditor_textChanged() {
    QString latexText = ui->latexEditor->toPlainText();
    if (latexText.length() == 0) return;
    if (! postThread->isRunning()){
        postThread->start();
    }

    postThread->mutex.lock();
    postThread->needUpdate = true;
    postThread->content = latexText;
    postThread->mutex.unlock();
    postThread->start();
    /*
    if (! postThread->isRunning()){
        qDebug("new update.");
        postThread->needUpdate = true;
        postThread->content = latexText;
        postThread->start();
    } else {
        DWORD nowTime = GetTickCount();
        if (nowTime - GV::threadTime > 5000) {
            GV::threadTime = nowTime;
            qDebug("not end.");
            postThread->terminate();
            postThread->needUpdate = true;
            postThread->content = latexText;
            postThread->start();
        }
    }
    */
    ui->latexPreviwer->setHtml(" ");
    /*
    char ip[] = "106.52.251.85";
    char type[] = "texToHtml";
    QByteArray transfer = latexText.toLatin1();
    std :: string latexResult = postFile(ip, 8888, type, transfer.data() , false);
    ui->latexPreviwer->setHtml(QString::fromStdString(latexResult));
    */
    //ui->latexPreviwer->setPlainText();
}
