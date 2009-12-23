#include "mainmenu.h"
#include "machconf.h"
#include "mainwindow.h"
#include "skymain.h"
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>

MainMenu::MainMenu(QWidget *parent)
        : QMenuBar(parent)
{
    createMenu();

}
bool MainMenu::maybeSave(){
    return true;
}

void MainMenu::openConf(){
    if (maybeSave()) {
         QString fileName = QFileDialog::getOpenFileName(this);
         if (!fileName.isEmpty()){
                bool valid = MachConf::parseConf(fileName);
                if(!valid){
                    QMessageBox::warning(this, tr("Error"),
                    tr("Config file is not valid format."));
                }
                MachConf * machConf = new MachConf(fileName);
                machConf->read();
                MainWindow *w = SkyMain::getMainWin();
                w->updateDevTreeView();
         }

     }
}

void MainMenu::openElf(){
    if (maybeSave()) {
         QString fileName = QFileDialog::getOpenFileName(this);
         if (!fileName.isEmpty()){
                bool valid = MachConf::parseConf(fileName);
                if(!valid){
                    QMessageBox::warning(this, tr("Error"),
                    tr("Config file is not valid format."));
                }
                MachConf * machConf = new MachConf(fileName);
                machConf->read();
                MainWindow *w = SkyMain::getMainWin();
                w->updateDevTreeView();
         }

     }
}

/*
 *
 */
void MainMenu::about(){
    QMessageBox::about(this, tr("About KSIM"),
             tr("The <b>KSIM</b> is an open source virtuailization platform based on "
                "SkyEye(www.skyeye.org).\n "
                "Author Michael.Kang(blackfin.kang@gmail.com)"));
}
void MainMenu::createMenu(){
    QMenu * fileMenu = this->addMenu(tr("&File"));
    QMenu * editMenu = this->addMenu(tr("&Edit"));
    //QMenu * logMenu = this->addMenu(tr("&Log"));
    QMenu * helpMenu = this->addMenu(tr("&About"));
    QAction *openConfAct;
    QAction *openElfAct ;
    QAction *exitAct ;
    QAction * runTimeConf ;
    QAction *helpAct ;
    QAction *aboutAct;

    openConfAct = new QAction(tr("&Open"), this);
    connect(openConfAct, SIGNAL(triggered()), this, SLOT(openConf()));
    fileMenu->addAction(openConfAct);

    /*openElfAct = new QAction(tr("&Create"), this);
    connect(openConfAct, SIGNAL(triggered()), this, SLOT(openElf()));
    fileMenu->addAction(openElfAct);
    */
    fileMenu->addSeparator();

    exitAct = new QAction(tr("&Exit"), this);
    fileMenu->addAction(exitAct);

    runTimeConf = new QAction(tr("&Preference"), this);
    editMenu->addAction(runTimeConf);

    helpAct = new QAction(tr("&Help"), this);
    helpMenu->addAction(helpAct);

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    helpMenu->addAction(aboutAct);
}

MainMenu::~MainMenu(){
}
