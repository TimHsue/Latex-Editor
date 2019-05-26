#include <Winsock2.h>
#include "login.h"
#include "ui_login.h"
#include <QCryptographicHash>
#include <cstdio>
#include <iostream>
#include <log.cpp>
#include <qmessagebox.h>

#define RQS_PACKAGE 256
#define RSP_PACKAGE 256
#define BOUNDARY "TPafZ#C3T"
#define COOKIE_LENGTH 16
#define BASIC_COOKIE "NOLOGIN"
#define ISNUM(X) (X <= '9' && X >= '0')

int GV::lastUpdate = 0;
int GV::threadTime = 0;
std :: string GV::cookie = "";

class Response {
public:
    int havsCss;
    std :: string html;
    std :: string css;

    Response() {}
    Response(std :: string html_, std :: string css_) {
        html = html_;
        css = css_;
        havsCss = true;
    }
    Response(std :: string html_) {
        html = html_;
        havsCss = false;
    }
};

std :: string postLog(char *addr, int serverPort, std :: string account, std :: string password,
                    bool isSignin = false, bool rqsVCode = false, int vCode = 0) {
    /* link to server*/
    SOCKET clientSocket;
    WSADATA wsaData;
    struct sockaddr_in serverAddr;

    std :: string header = "";
    char sndBuff[RQS_PACKAGE + 1], rcvBuff[RSP_PACKAGE + 1];
    memset(sndBuff, 0, sizeof(sndBuff));
    memset(rcvBuff, 0, sizeof(rcvBuff));

    WSAStartup(MAKEWORD(2, 1), &wsaData);

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR("failed to start socket!");
        return BASIC_COOKIE;
    } else {
        LOG("socket started.");
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = ::htons(serverPort);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(addr);

    if (::connect(clientSocket, (struct sockaddr *)&serverAddr,
                  sizeof(serverAddr)) < 0) {
        ERR("failed to connect to socket server!");
        return BASIC_COOKIE;
    } else {
        LOG("socket connected.");
    }

    /* gen header */
    header = "";
    header += "POST http://";
    header += addr;
    header += ":";
    header += std :: to_string(serverPort);
    header += "?";
    if (! isSignin) {
        header += "login";
    } else {
        if (rqsVCode) {
            header += "vcode";
        } else {
            header += "signin";
        }
    }
    header += " HTTP/1.0\r\n";

    header += "Host: ";
    header += addr;
    header += ":";
    header += std :: to_string(serverPort);
    header += "\r\n";

    header += "Connection: keep-alive\r\n";

    header += "Accept: */*\r\n";

    if (! isSignin) {
        header += "Content-Type: text=2\r\n";
    } else {
        if (rqsVCode) {
            header += "Content-Type: text=1\r\n";
        } else {
            header += "Content-Type: text=3\r\n";
        }
    }

    header += "\r\n";

    header += "actSize=" + std :: to_string(account.length());
    header += "\r\n";

    if (! isSignin) {
        header += "pwdSize=" + std :: to_string(password.length());
        header += "\r\n";
    } else if (! rqsVCode) {
        header += "pwdSize=" + std :: to_string(password.length());
        header += "\r\n";
    }

    while (header.length() <= RQS_PACKAGE) header += "-";

    LOG(header);

    /* send header */
    int reciveLength = 0;
    if ((reciveLength = send(clientSocket, header.c_str(), RQS_PACKAGE, 0)) < 0) {
        ERR("failed to send request header!");
        return BASIC_COOKIE;
    } else {
        LOG("request sent.");
    }

    /* send act */
    LOG(account);
    if ((reciveLength = send(clientSocket, account.c_str(), account.length(), 0)) < 0) {
        ERR("failed to send account!");
        return BASIC_COOKIE;
    } else {
        LOG("request sent.");
    }

    if (! isSignin) {
        /* send pwd */
        LOG(password);
        if ((reciveLength = send(clientSocket, password.c_str(), password.length(), 0)) < 0) {
            ERR("failed to send pwd!");
            return BASIC_COOKIE;
        } else {
            LOG("request sent.");
        }
    } else if (! rqsVCode) {
        /* send pwd */
        LOG(password);
        if ((reciveLength = send(clientSocket, password.c_str(), password.length(), 0)) < 0) {
            ERR("failed to send pwd!");
            return BASIC_COOKIE;
        } else {
            LOG("request sent.");
        }

        std :: string vCodeString = std :: to_string(vCode);
        /* send vcode */
        if ((reciveLength = send(clientSocket, vCodeString.c_str(), 7, 0)) < 0) {
            ERR("failed to send vcode!");
            return BASIC_COOKIE;
        } else {
            LOG("request sent.");
        }
    }

    if (! rqsVCode) {
        /* recerve cookie */
        if ((reciveLength = recv(clientSocket, rcvBuff, COOKIE_LENGTH, 0)) < 0) {
            ERR("failed to receive cookie!");
            return BASIC_COOKIE;
        } else if (reciveLength > 0) {
            rcvBuff[COOKIE_LENGTH] = 0;
            LOG(rcvBuff);

            Sleep(1);
            closesocket(clientSocket);
            WSACleanup();
            return rcvBuff;
        }
    } else {

        Sleep(1);
        closesocket(clientSocket);
        WSACleanup();
        return "Waiting for vcode";
    }


    Sleep(1);
    closesocket(clientSocket);
    WSACleanup();

    return BASIC_COOKIE;
}


login::login(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::login) {
        ui->setupUi(this);
        strcpy_s(ip, "106.52.251.85");
        strcpy_s(type, "texToHtml");
    }

login::~login() {
    delete ui;
}

void login::on_loginButton_clicked() {
    QString account, password;

    account = ui->account->text();
    password = ui->password->text();
    password += "latexovo";

    QString md5Res;
    QByteArray passwordByte, md5Byte;
    QCryptographicHash md5Gen(QCryptographicHash::Md5);
    passwordByte.append(password);
    md5Gen.addData(passwordByte);
    md5Byte = md5Gen.result();
    md5Res.append(md5Byte.toHex());

    QByteArray accountBA = account.toLatin1();
    QByteArray passwordBA = md5Res.toLatin1();

    std :: string cookie =
            postLog(ip, 8888, accountBA.data(), passwordBA.data());
    if (cookie.compare("network goes wrg")) {

    } else if (cookie.compare("wrong act or pwd")) {

    } else {
// Todo: save cookie
        GV::cookie = cookie;
        this->close();
    }
}

void login::on_signButton_clicked() {
    QString account, password, vCode;

    account = ui->account->text();
    password = ui->password->text();
    vCode = ui->vCode->text();

    password += "latexovo";

    QString md5Res;
    QByteArray passwordByte, md5Byte;
    QCryptographicHash md5Gen(QCryptographicHash::Md5);
    passwordByte.append(password);
    md5Gen.addData(passwordByte);
    md5Byte = md5Gen.result();
    md5Res.append(md5Byte.toHex());

    QByteArray accountBA = account.toLatin1();
    QByteArray passwordBA = md5Res.toLatin1();

    std :: string cookie =
            postLog(ip, 8888, accountBA.data(), passwordBA.data(),
                    true, false, vCode.toInt());
    if (cookie.compare("network goes wrg")) {
        QMessageBox::information(NULL, "Error", "network goes wrong",
                                 QMessageBox::Ok, QMessageBox::Ok);
    } else if (cookie.compare("wrong act or pwd")) {
        QMessageBox::information(NULL, "Wrong", "Wrong account or password",
                                 QMessageBox::Ok, QMessageBox::Ok);
    } else {
// Todo: save cookie
        GV::cookie = cookie;
        this->close();
    }
}

void login::on_vCodeButton_clicked() {
    QString account;
    account = ui->account->text();

    bool isPhone = true;
    for (auto i : account)
        if (! ISNUM(i)) isPhone = false;

    if (account.length() != 11 || !isPhone) {
        QMessageBox::information(NULL, "Error", "wrong phone number",
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QByteArray accountBA = account.toLatin1();

    std :: string cookie =
            postLog(ip, 8888, accountBA.data(), "NULL", true, true);
    QMessageBox::information(NULL, "Info", "please check your phone",
                             QMessageBox::Ok, QMessageBox::Ok);
}
