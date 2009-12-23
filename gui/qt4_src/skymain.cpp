#include "skymain.h"

SkyMain::SkyMain()
{

}

MainWindow * SkyMain::getMainWin(){
    static MainWindow *w;
    if(!w)
        w = new MainWindow();
    return w;
}

Console * SkyMain::getUartConsole(){
    MainWindow *w = SkyMain::getMainWin();    
    return w->getUartConsole();
}

DevTreeView * SkyMain::getDevTreeView(){
    MainWindow *w = SkyMain::getMainWin();

    return w->getDevTreeView();
}
