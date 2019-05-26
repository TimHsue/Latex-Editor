#include "login.h"
#include "ui_login.h"
#include "post.cpp"
#include <QCryptographicHash>

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
    password = ui->account->text();
    password += "latexovo";

    QString md5Res;
    QByteArray passwordByte, md5Byte;
    QCryptographicHash md5Gen(QCryptographicHash::Md5);
    passwordByte.append(password);
    md5Gen.addData(passwordByte);
    md5Byte = md5Gen.result();
    md5Res.append(md5Byte.toHex());

    postLog(ip, 8888, account, password);
}
