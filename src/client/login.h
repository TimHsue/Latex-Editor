#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>

namespace Ui {
    class login;
}

class login : public QMainWindow {
    Q_OBJECT

private:
    char ip[16];
    char type[16];

public:
    explicit login(QWidget *parent = nullptr);
    ~login();

private slots:
    void on_loginButton_clicked();

private:
    Ui::login *ui;
};

#endif // LOGIN_H
