#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>

class GV {
public:
    static int lastUpdate;
    static int threadTime;
    static std :: string cookie;
};

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

    void on_signButton_clicked();

    void on_vCodeButton_clicked();

private:
    Ui::login *ui;
};

#endif // LOGIN_H
