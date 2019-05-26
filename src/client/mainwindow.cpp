#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "postthread.h"
#include "log.cpp"
#include <QDebug>

DWORD GV::lastUpdate = 0;
DWORD GV::threadTime = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
        ui->setupUi(this);
        postThread = new PostThread(this);
        loginClient = new login(this);
        QTextCursor *textCursor = new QTextCursor(ui->latexEditor->document());
        textCursor->insertText("\\documentclass{article}\n");
        textCursor->insertText("\\begin{document}\n\n");
        textCursor->insertText("\\end{document}");
        textCursor->setPosition(41);
        ui->latexEditor->setTextCursor(*textCursor);
    }

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateUI(QString latexHTML, QString latexCSS) {
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
}

void MainWindow::on_textbf_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "textbf{";
        newText += slecetedText + "}";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("\\textbf{}");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_emph_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "textsl{";
        newText += slecetedText + "}";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("\\textsl{}");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_underline_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "underline{";
        newText += slecetedText + "}";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("\\underline{}");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_login_triggered() {
    loginClient->show();
}
