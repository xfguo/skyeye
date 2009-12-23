#ifndef SKYMAIN_H
#define SKYMAIN_H

#include <QObject>
#include "mainwindow.h"

class SkyMain : public QObject
{
    Q_OBJECT
public:
    SkyMain();
    static Console * getUartConsole();
    static DevTreeView * getDevTreeView();
    static MainWindow * getMainWin();
private:

};

#endif // SKYMAIN_H
