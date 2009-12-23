#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "mainmenu.h"
#include "sim_control.h"
#include <QToolButton>
#include <QIcon>
#include <QSplitter>
#include <QTreeView>
#include <QAction>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    toolBar = new QToolBar();
    //QAction* startAct = new QAction(QIcon("./images/start_16px.png"), tr("&Start"), this);
    //openAct->setShortcut("Ctrl + O");
    //startAct->setShortcut("Ctrl+O");
    //startAct->setToolTip("Start virtual machine");
    //connect(startAct, SIGNAL(triggered()), this, SLOT(startMachine()));
    QToolButton* start_button = new QToolButton();
    QToolButton* stop_button = new QToolButton();
    //QIcon * start_icon = new QIcon("./images/start_16px.png");
    //start_button->setIcon(QIcon("./images/state_running_16px.png"));
    start_button->setIcon(QIcon("./images/start_16px.png"));
    start_button->setToolTip("Start virtual machine.");

    stop_button->setIcon(QIcon("./images/state_powered_off_16px.png"));
    stop_button->setToolTip("Stop virtual machine.");
    toolBar->addWidget(start_button);
    toolBar->addWidget(stop_button);


    uartConsole = new Console();
    uartConsole->setReadOnly(true);
    //uartConsole->appendPlainText("asdasd");

    devTreeView = new DevTreeView(QString("machine list"));
    splitter = new QSplitter(this);

    splitter->addWidget(devTreeView);
    splitter->addWidget(uartConsole);



    this->setCentralWidget(splitter);

    this->addToolBar(toolBar);

    MainMenu *mainMenu = new MainMenu();

    this->setMenuBar(mainMenu);

    //resize(480, 320);
    this->setGeometry(400,400,640, 480);
    this->setWindowTitle("SKYEYE GUI");

    SIM_init();

}

Console * MainWindow::getUartConsole(){
    return uartConsole;
}

DevTreeView * MainWindow::getDevTreeView(){
    return devTreeView;
}

void MainWindow::updateDevTreeView(){
}

void MainWindow::startMachine(){
    //SIM_start();
}

MainWindow::~MainWindow()
{

}
