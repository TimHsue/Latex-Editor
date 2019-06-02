#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "postthread.h"
#include "log.cpp"
#include "login.h"
#include "savefile.h"
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>

#define RQS_PACKAGE 256
#define RSP_PACKAGE 256
#define ISNUM(X) (X <= '9' && X >= '0')

MainWindow::MainWindow(QWidget *parent) :
                       QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setWindowTitle("T-LatexEditor");

    postThread = new PostThread(this);
    loginClient = new login(this);
    loginClient->setWindowModality(Qt::ApplicationModal);

    QTextCursor *textCursor = new QTextCursor(ui->latexEditor->document());
    textCursor->insertText("\\documentclass{article}\n");
    textCursor->insertText("\\title{newLatex}\n");
    textCursor->insertText("\\author{Tim}\n");
    textCursor->insertText("\\begin{document}\n");
    textCursor->insertText("\\maketitle\n\n");

    textCursor->insertText("\\end{document}");
    textCursor->setPosition(82);
    ui->latexEditor->setTextCursor(*textCursor);

    defaultSavePath = QDir::currentPath();
    defaultSaveName = "newLatex";
    defaultSaveType = "";
    isSaved = true;
    multiUserOn = false;
    saveFile = new SaveFile(this, this);
    saveFile->setWindowModality(Qt::ApplicationModal);

    qtimer = new QTimer(this);
    connect(qtimer, SIGNAL(timeout()), this, SLOT(downloadCloudSlot()));
    connect(qtimer, SIGNAL(timeout()), this, SLOT(uploadCloudSlot()));
}

void MainWindow::downloadCloudSlot() {
    downloadCloud("tmptex970830");
}

void MainWindow::uploadCloudSlot() {
    uploadCloud("tmptex970830");
}


MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateUI(QString latexHTML) {
    ui->latexPreviwer->setHtml(latexHTML);
}

void MainWindow::on_latexEditor_textChanged() {
    isSaved = false;
    QString latexText = ui->latexEditor->toPlainText();
    if (latexText.length() == 0) return;
    if (! postThread->isRunning()){
        postThread->start();
    }


    postThread->mutex.lock();
    postThread->needUpdate = true;
    postThread->content = latexText;
    postThread->mutex.unlock();
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

void MainWindow::on_getFile_triggered() {
    saveFile->show();
}

void MainWindow::saveFileTo(QByteArray content) {
    if (defaultSaveType == "tex") {
        QString contentQstring = ui->latexEditor->toPlainText();
        content = contentQstring.toLatin1();
        isSaved = true;
    }

    QDir qdir(defaultSavePath);
    QString fileName = defaultSaveName + "." + defaultSaveType;

    if (qdir.exists(fileName)) {
        if (changed) {
            QMessageBox message(QMessageBox::NoIcon, "Warning",
                                "File already existed.\nAre you sure to cover it?",
                                QMessageBox::Yes | QMessageBox::No, NULL);
            if (message.exec() == QMessageBox::No) {
                return ;
            }
        }
    }

    QFile file(defaultSavePath + "/" + fileName);



    if (file.open(QIODevice::WriteOnly)) {

        for (int i = 0; i < content.length(); i++) {
            int tmp = content[i];
        }
        file.write(content);
        file.close();
    } else {
        qDebug() << "failed to open file";
    }
}

QString MainWindow::getContent() {
    QString latexText = ui->latexEditor->toPlainText();
    if (latexText.length() == 0) return "None";
    return latexText;
}

void MainWindow::on_openFile_triggered() {
    if (isSaved == false) {
        QMessageBox message(QMessageBox::NoIcon, "Warning",
                            "File changes havent been saved.\nDo you want to save it?",
                            QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes) {
            saveFile->show();
            return;
        }
    }

    QString path =
            QDir::toNativeSeparators(
                QFileDialog::getOpenFileName(
                    this, tr("Open file"), ".",
                    tr("Latex Files(*.tex)")));
    if(path.length() == 0) {
        QMessageBox::information(NULL, tr("Path"),
                                 tr("You didn't select any files."));
        return;
    }

    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString text = file.readAll();
    ui->latexEditor->setText(text);
    file.close();
    isSaved = true;
}

void MainWindow::on_upload_triggered() {
    bool onYes = false;
    QString text = QInputDialog::getText(this, tr("Input File Name"),
            tr("File name:"), QLineEdit::Normal,
            "myLatex", &onYes);
    if (onYes && !text.isEmpty()) {

    }
}

void MainWindow::on_section_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "section{";
        newText += slecetedText + "}";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("\\section{}");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}


void MainWindow::on_subsection_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "subsection{";
        newText += slecetedText + "}";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("\\subsection{}");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_subsubsection_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "subsubsection{";
        newText += slecetedText + "}";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("\\subsubsection{}");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_paragraph_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "paragraph{";
        newText += slecetedText + "}";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("\\paragraph{}");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_tableofcontents_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    textCursor.insertText("\\tableofcontents");
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_lineFormula_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "$";
        newText += slecetedText + "$";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("$$");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_paragraphFormula_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\n$$";
        newText += slecetedText + "$$";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("$$\n\n$$");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 3);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_label_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "label{";
        newText += slecetedText + "}";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("\\label{}");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_newLine_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    textCursor.insertText("\n\\newline\n");
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_newPage_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    textCursor.insertText("\n\\newpage\n");
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_quato_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "begin{quote}\n";
        newText += slecetedText + "\n";
        newText += "\\";
        newText += "end{quote}";
        textCursor.insertText(newText);
    } else {
        QString newText = "\\";
        newText += "begin{quote}\n\n";
        newText += "\\";
        newText += "end{quote}";
        textCursor.insertText(newText);
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 12);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_makeCenter_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "begin{center}\n";
        newText += slecetedText + "\n";
        newText += "\\";
        newText += "end{center}";
        textCursor.insertText(newText);
    } else {
        QString newText = "\\";
        newText += "begin{center}\n\n";
        newText += "\\";
        newText += "end{center}";
        textCursor.insertText(newText);
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 13);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_footnote_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "footnote{";
        newText += slecetedText + "}";
        textCursor.insertText(newText);
    } else {
        textCursor.insertText("\\footnote{}");
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 1);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_flushleft_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "begin{flushleft}\n";
        newText += slecetedText + "\n";
        newText += "\\";
        newText += "end{flushleft}";
        textCursor.insertText(newText);
    } else {
        QString newText = "\\";
        newText += "begin{flushleft}\n\n";
        newText += "\\";
        newText += "end{flushleft}";
        textCursor.insertText(newText);
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 16);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::on_flushright_triggered() {
    QTextCursor textCursor = ui->latexEditor->textCursor();
    if (textCursor.hasSelection()) {
        QString slecetedText = textCursor.selectedText();
        QString newText = "\\";
        newText += "begin{flushright}\n";
        newText += slecetedText + "\n";
        newText += "\\";
        newText += "end{flushright}";
        textCursor.insertText(newText);
    } else {
        QString newText = "\\";
        newText += "begin{flushright}\n\n";
        newText += "\\";
        newText += "end{flushright}";
        textCursor.insertText(newText);
        textCursor.movePosition(QTextCursor::Left,
                                QTextCursor::MoveAnchor, 17);
    }
    ui->latexEditor->setTextCursor(textCursor);
}

void MainWindow::uploadCloud(QString fileName) {
    QString latexText = ui->latexEditor->toPlainText();
    if (latexText.length() == 0) return;
    PostThread *uploadThread = new PostThread(this);
    uploadThread->isDownload = false;
    uploadThread->isUpload = true;
    uploadThread->content = latexText;
    uploadThread->pCookie = QString::fromStdString(GV::cookie);
    uploadThread->fileName = fileName;
    uploadThread->start();
}

void MainWindow::on_uploadCloud_triggered() {
    if (GV::cookie.length() <= 1) {
        QMessageBox::information(NULL, "Error", "You need to login first.",
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    bool isOK = false;
    QString fileName =
        QInputDialog::getText(this, tr("File Name"), tr("file name"),
                              QLineEdit::Normal, "newLatex", &isOK);
    if ((!isOK) || (fileName.length() < 1)) {
        QMessageBox::information(NULL, "Error", "please input at least one character.",
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    uploadCloud(fileName);
}

void MainWindow::uploadResult(QString info) {
    QMessageBox::information(NULL, "Info", info,
                             QMessageBox::Ok, QMessageBox::Ok);
}

std :: string getSqlContent(char *addr, int serverPort, const char *target,
                            std :: string pCookie = "",
                            std :: string name = "") {
    SOCKET clientSocket;
    WSADATA wsaData;
    struct sockaddr_in serverAddr;

    std :: string header;
    std :: string result = "";
    char sndBuff[RQS_PACKAGE + 1], rcvBuff[RSP_PACKAGE + 1];
    memset(sndBuff, 0, sizeof(sndBuff));
    memset(rcvBuff, 0, sizeof(rcvBuff));

    WSAStartup(MAKEWORD(2, 1), &wsaData);

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR("failed to start socket!");
        WSACleanup();
        return "failed to start socket!";
    } else {
        LOG("socket started.");
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = ::htons(serverPort);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(addr);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr,
                sizeof(serverAddr)) < 0) {
        ERR("failed to connect to socket server!");
        closesocket(clientSocket);
        WSACleanup();
        return "failed to connect to socket server!";
    } else {
        LOG("socket connected.");
    }

    header = "";
    header += "POST http://";
    header += addr;
    header += ":";
    header += std :: to_string(serverPort);
    header += "?";
    header += target;
    header += " HTTP/1.0\r\n";

    header += "Host: ";
    header += addr;
    header += ":";
    header += std :: to_string(serverPort);
    header += "\r\n";

    header += "Connection: keep-alive\r\n";

    header += "Accept: */*\r\n";

    header += "Content-Type: file\r\n";

    if (pCookie.length() > 1) {
        header += "cookie=";
        header += pCookie + "\r\n";
    }

    if (name.length() > 1) {
        header += "name=";
        header += name + "\r\n";
    }

    LOG(header);

    while (header.length() < RQS_PACKAGE) header += " ";

    int reciveLength = 0;
    if ((reciveLength = send(clientSocket, header.c_str(), RQS_PACKAGE, 0)) < 0) {
        ERR("failed to send request header!");
        closesocket(clientSocket);
        WSACleanup();
        return "failed to send request header!";
    } else {
        LOG("request sent.");
    }

    if ((reciveLength = recv(clientSocket, rcvBuff, RSP_PACKAGE, 0)) < 0) {
        ERR("failed to receive response header!");
        closesocket(clientSocket);
        WSACleanup();
        return "failed to receive response!";
    } else if (reciveLength > 0) {
        rcvBuff[RSP_PACKAGE] = 0;
        std :: string rspHeader = rcvBuff;
        int sizePos = 0;
        int dataSize = 0;

        sizePos = rspHeader.find("dataSize=");
        if (sizePos < 0) {
            closesocket(clientSocket);
            WSACleanup();
            return "response header error";
        } else {
            sizePos += 9;
            while (ISNUM(rspHeader[sizePos])) {
                dataSize = 10 * dataSize + rspHeader[sizePos] - '0';
                sizePos++;

                if (sizePos > rspHeader.length()) break;
            }
        }

        if (dataSize == 0) {
            closesocket(clientSocket);
            WSACleanup();
            return "dataSize error!";
        }

        memset(rcvBuff, 0, sizeof(rcvBuff));
        for (int i = 0; i < dataSize; i += RSP_PACKAGE) {
            int receiveSize = RSP_PACKAGE;
            if (i + RSP_PACKAGE > dataSize) {
                receiveSize = dataSize - i;
            }

            if ((reciveLength = recv(clientSocket, rcvBuff, receiveSize, 0)) < 0) {
                ERR("failed to receive sql content!");
                closesocket(clientSocket);
                WSACleanup();
                return "failed to receive sql content!";
            } else {
                rcvBuff[receiveSize] = 0;
                result += rcvBuff;
                memset(rcvBuff, 0, sizeof(rcvBuff));
            }
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    return result;
}

void MainWindow::downloadCloud(QString fileName) {
    char ip[16];
    char type[16];
    strcpy_s(ip, "106.52.251.85");
    strcpy_s(type, "downloadTex");
    QByteArray nameBA = fileName.toLatin1();

    std :: string result = getSqlContent(ip, 8888, type,
                                         GV::cookie, nameBA.data());

    QString text = QString::fromStdString(result);
    ui->latexEditor->setText(text);
    isSaved = true;
}

void MainWindow::on_loadCloud_triggered() {
    if (GV::cookie.length() <= 1) {
        QMessageBox::information(NULL, "Error", "You need to login first.",
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    bool isOK = false;
    QString fileName =
        QInputDialog::getText(this, tr("File Name"), tr("file name"),
                              QLineEdit::Normal, "", &isOK);
    if ((!isOK) || (fileName.length() < 1)) {
        QMessageBox::information(NULL, "Error", "please input at least one character.",
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    if (!isSaved) {
        QMessageBox message(QMessageBox::NoIcon, "Warning",
                            "File changes haven't been saved.\nDo you want to save it?",
                            QMessageBox::Yes | QMessageBox::No, NULL);
        if (message.exec() == QMessageBox::Yes) {
            saveFile->show();
            return;
        }
    }

    downloadCloud(fileName);
}

void MainWindow::on_multiuser_triggered() {
    if (multiUserOn == false) {
        uploadCloud("tmptex970830");
        qtimer->start(1000);
    } else {
        qtimer->stop();
    }
    multiUserOn ^= 1;
}
