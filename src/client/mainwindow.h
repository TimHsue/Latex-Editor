#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <Windows.h>
#include <QTextCursor>
#include "login.h"

namespace Ui {
    class MainWindow;
}

class PostThread;
class login;
class SaveFile;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString defaultSavePath;
    QString defaultSaveName;
    QString defaultSaveType;
    bool changed;

    QString getContent();

    // QString getDefaultSavePath();

    // void setDefaultSavePath(QString path);

public slots:
    void updateUI(QString latexHTML);

    void saveFileTo(QByteArray path);

    void uploadResult(QString info);

private slots:
    void on_latexEditor_textChanged();

    void on_textbf_triggered();

    void on_emph_triggered();

    void on_underline_triggered();

    void on_login_triggered();

    void on_getFile_triggered();

    void on_openFile_triggered();

    void on_upload_triggered();

    void on_section_triggered();

    void on_subsection_triggered();

    void on_subsubsection_triggered();

    void on_paragraph_triggered();

    void on_tableofcontents_triggered();

    void on_lineFormula_triggered();

    void on_paragraphFormula_triggered();

    void on_label_triggered();

    void on_newLine_triggered();

    void on_newPage_triggered();

    void on_quato_triggered();

    void on_makeCenter_triggered();

    void on_footnote_triggered();

    void on_flushleft_triggered();

    void on_flushright_triggered();

    void on_uploadCloud_triggered();

    void on_loadCloud_triggered();

private:
    Ui::MainWindow *ui;
    PostThread *postThread;
    QTextCursor textCursor;
    login *loginClient;
    SaveFile *saveFile;

    bool isSaved;
};

#endif // MAINWINDOW_H
