#include "welcomescreen.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDebug>

WelcomeScreen::WelcomeScreen(QWidget* parent, MainWindow *mainWindow) : QWidget(parent){
    this->mainWindow=mainWindow;
    process->setWorkingDirectory("/home/plag/Projects/networkVisualiser/backend");
    QString Command= "./listDevices.sh";
    process->start(Command);
    process->waitForFinished();
    QString StdOut = process->readAllStandardOutput();
    QString StdError = process->readAllStandardError();
    qDebug()<<"Standard Out: "<< StdOut.toStdString() << "\nStandard Error: "<< StdError;
    setupUi();
}

void WelcomeScreen::setupUi(){
    auto *mainLayout = new QVBoxLayout(this);
    //QLabel* Heading = new QLabel(StdOut);
    //QLabel* HeadingError = new QLabel(StdError);
    QHBoxLayout *input = new QHBoxLayout;
    QLabel * text = new QLabel("Select your network device: ");
    QComboBox * devices = new QComboBox;
    devices->addItem("lo");
    devices->addItem("eth0");
    devices->addItem("wlan0");
    //mainLayout->addWidget(Heading);
    //mainLayout->addWidget(HeadingError);
    input->addWidget(text);
    input->addWidget(devices);
    QHBoxLayout *buttons = new QHBoxLayout;
    QPushButton* submit = new QPushButton("OK", this);
    buttons->addWidget(submit);
    mainLayout->addLayout(input);
    mainLayout->addLayout(buttons);
    connect(submit, &QPushButton::clicked, this, &WelcomeScreen::showMainWindow);
    //setCentralWidget(mainLayout);

}

void WelcomeScreen::showMainWindow(){
    process->startDetached("sudo ./packet_sniffer wlp0s20f3 | python3 app.py");
    mainWindow->show();
    //system("cd /home/plag/Projects/networkVisualiser/backend");
    //system("sudo /home/plag/Projects/networkVisualiser/backend/packet_sniffer wlp0s20f3 | python3 /home/plag/Projects/networkVisualiser/backendapp.py");
    this->hide();
}
