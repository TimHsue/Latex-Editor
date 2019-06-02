#include "savefile.h"
#include "ui_savefile.h"
#include <QDir>
#include <QFileDiaLog>
#include <QDebug>
#include "postthread.h"

SaveFile::SaveFile(QWidget *parent, MainWindow *mainWindow_) :
                   QMainWindow(parent), ui(new Ui::SaveFile) {
    ui->setupUi(this);
    this->setWindowTitle("Save as...");
    mainWindow = mainWindow_;

    ui->selectDirector->addItem(mainWindow->defaultSavePath);
    ui->selectType->addItem("Latex Source(.tex)");
    ui->selectType->addItem("HTML Source(.html)");
    ui->selectType->addItem("PDF Document(.pdf)");
    ui->fileName->clear();
    ui->fileName->insert(mainWindow_->defaultSaveName);

    connect(this, SIGNAL(saveFileGot(QByteArray)),
            mainWindow, SLOT(saveFileTo(QByteArray)));
}

SaveFile::~SaveFile() {
    delete ui;
}

void SaveFile::on_showDirector_clicked() {
    QString director =
            QDir::toNativeSeparators(QFileDialog::getExistingDirectory(
                                         this, tr("Save path"),
                                         QDir::currentPath()));
    if (! director.isEmpty()) {
        if (ui->selectDirector->findText(director) == -1) {
            ui->selectDirector->addItem(director);
        }
        ui->selectDirector->setCurrentIndex(ui->selectDirector->findText(director));
    }
}

void SaveFile::on_saveButton_clicked() {
    QString path = ui->selectDirector->currentText();
    QString type = ui->selectType->currentText();
    QString name = ui->fileName->text();

    if (type.compare("Latex Source(.tex)") == 0) type = "tex";
    if (type.compare("HTML Source(.html)") == 0) type = "html";
    if (type.compare("PDF Document(.pdf)") == 0) type = "pdf";

    if (path == "") return;
    if (name == "") name = "newLatex";

    QString lastSave = mainWindow->defaultSavePath +
                       mainWindow->defaultSaveName +
                       mainWindow->defaultSaveType;
    QString newSave = path + name + type;

    if (lastSave.compare(newSave) == 0) mainWindow->changed = false;
    else mainWindow->changed = true;

    mainWindow->defaultSavePath = path;
    mainWindow->defaultSaveName = name;
    mainWindow->defaultSaveType = type;

    if (type.compare("tex") == 0) {
        emit saveFileGot(QByteArray(" ", 1));
        this->close();
        return;
    }

    /* creat thread to save */

    PostThread *postThread;
    postThread = new PostThread(mainWindow);
    postThread->isDownload = true;
    postThread->isUpload = false;
    postThread->content = mainWindow->getContent();
    postThread->isPdf = type.compare("html");
    postThread->start();

    this->close();
}
