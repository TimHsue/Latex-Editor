#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "postthread.h"
#include "log.cpp"

DWORD GV::lastUpdate = 0;

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

    postThread->mutex.lock();
    postThread->needUpdate = true;
    postThread->content = latexText;
    postThread->mutex.unlock();


    postThread->start();

    /*
    char ip[] = "106.52.251.85";
    char type[] = "texToHtml";
    QByteArray transfer = latexText.toLatin1();
    std :: string latexResult = postFile(ip, 8888, type, transfer.data() , false);
    ui->latexPreviwer->setHtml(QString::fromStdString(latexResult));
    */
    //ui->latexPreviwer->setPlainText();
}
