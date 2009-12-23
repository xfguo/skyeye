#include <iostream>

#include "machconf.h"
#include "console.h"
#include "skymain.h"
#include <QFile>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QMessageBox>
#include <QIODevice>


MachConf::MachConf()
{
}
MachConf::MachConf(QString fileName)
{
    _fileName = fileName;
}
bool MachConf::parseConf(QString fileName){
   QFile * confFile = new QFile(fileName);
    QXmlSimpleReader xmlReader;
    QXmlInputSource *source = new QXmlInputSource(confFile);
    bool valid = xmlReader.parse(source);
    if(!valid){
        //std::cout<<"Parse error"<<std::endl;
        //QString* title = new QString("asd");
        //QString* msg = new QString("fghfghfgh");
        //QMessageBox::warning(this,title, msg);
        /*
         QMessageBox::about(this, tr("About KSIM"),
             tr("The <b>KSIM</b> is an open source virtuailization platform based on "
                "SkyEye(www.skyeye.org).\n "
                "Author Michael.Kang(blackfin.kang@gmail.com)"));
                */
        return false;
    }
    return true;
}

bool MachConf::read()
 {
     QString errorStr;
     int errorLine;
     int errorColumn;
     QFile *device = new QFile(_fileName);
     if (!device->open(QIODevice::ReadOnly))
        return false;
     if (!domDocument.setContent(device, true, &errorStr, &errorLine,
                                 &errorColumn)) {

         return false;
     }

     QDomElement root = domDocument.documentElement();

    //Console *console =
     //cout <<
     //std::basic_ostream<<root.tagName()<<endl;

    Console * console = SkyMain::getUartConsole();
    DevTreeView *treeView = SkyMain::getDevTreeView();

    //QDomElement child = root.firstChildElement("arch");
    if(root.tagName() != "machine"){        
         return false;
    }
    else{
        QDomAttr attr = root.attributeNode(QString("name"));
        treeView->insertMachine(attr.value());
    }
    QDomElement board = root.firstChildElement("board");

    //console->insertPlainText(attr.value());
    console->insertPlainText("\n");
    //console->insertPlainText(child);

    //console->insertPlainText(QString("ljklkjkejlwer"));

       /*
     if (root.tagName() != "xbel") {
         QMessageBox::information(window(), tr("DOM Bookmarks"),
                                  tr("The file is not an XBEL file."));
         return false;
     } else if (root.hasAttribute("version")
                && root.attribute("version") != "1.0") {
         QMessageBox::information(window(), tr("DOM Bookmarks"),
                                  tr("The file is not an XBEL version 1.0 "
                                     "file."));
         return false;
     }

     clear();

     disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
                this, SLOT(updateDomElement(QTreeWidgetItem *, int)));

     QDomElement child = root.firstChildElement("folder");
     while (!child.isNull()) {
         parseFolderElement(child);
         child = child.nextSiblingElement("folder");
     }

     connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
             this, SLOT(updateDomElement(QTreeWidgetItem *, int)));
    */
     return true;
}

QDomElement getChild(QDomElement * e, QString name){
    QDomElement child = e->firstChildElement(name);
    return child;
}
