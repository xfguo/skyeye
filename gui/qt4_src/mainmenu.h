#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMenuBar>

class MainMenu : public QMenuBar
{
    Q_OBJECT

public:
    MainMenu(QWidget *parent = 0);
    ~MainMenu();

private slots:
    void about();
    void openConf();
    void openElf();

private:
    bool maybeSave();
    void createMenu();
};

#endif // MAINMENU_H
