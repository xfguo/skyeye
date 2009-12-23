#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QSplitter>
#include "console.h"
#include "devtreeview.h"

namespace Ui
{
    class MainWindowClass;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);    
    ~MainWindow();
    Console * getUartConsole();
    DevTreeView * getDevTreeView();
    void updateDevTreeView();


private:
    Ui::MainWindowClass *ui;
    Console * uartConsole;
    QSplitter *splitter;
    QToolBar * toolBar;
    DevTreeView * devTreeView;
    void startMachine();
};

#endif // MAINWINDOW_H
