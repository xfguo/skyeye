#include "devtreeview.h"
#include <QTreeWidgetItem>

DevTreeView::DevTreeView()
{

}
DevTreeView::DevTreeView(QString rootName)
{
    QTreeWidgetItem * item = new QTreeWidgetItem(this);
    item->setText(0, rootName);
    this->insertTopLevelItem(0, item);
}

void DevTreeView::insertMachine(QString machName){
    /*
    QTreeWidgetItem * machItem = topLevelItem(0);
    QString rootName = machItem->text(0);
    QString machine = QString("machine");
    if(rootName.compare(machine)){
        this->takeTopLevelItem(0);
        QTreeWidgetItem * item = new QTreeWidgetItem(this);
        item->setText(0, machName);
        this->insertTopLevelItem(0, item);
    }
    */
    QTreeWidgetItem * machItem = topLevelItem(0);
    QTreeWidgetItem * item = new QTreeWidgetItem(this);
    item->setText(0, machName);
    machItem->insertChild(0, item);
}
