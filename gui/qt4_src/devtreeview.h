#ifndef DEVTREEVIEW_H
#define DEVTREEVIEW_H

#include <QTreeWidget>

class DevTreeView : public QTreeWidget
{
public:
    DevTreeView();
    DevTreeView(QString rootName);
    void insertMachine(QString machName);
};

#endif // DEVTREEVIEW_H
