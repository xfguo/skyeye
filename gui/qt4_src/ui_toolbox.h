/********************************************************************************
** Form generated from reading ui file 'toolbox.ui'
**
** Created: Sun Mar 15 21:01:55 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_TOOLBOX_H
#define UI_TOOLBOX_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QToolBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ToolBox
{
public:
    QWidget *page;
    QWidget *page1;

    void setupUi(QToolBox *ToolBox)
    {
        if (ToolBox->objectName().isEmpty())
            ToolBox->setObjectName(QString::fromUtf8("ToolBox"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        ToolBox->addItem(page, QString::fromUtf8("Page 1"));
        page1 = new QWidget();
        page1->setObjectName(QString::fromUtf8("page1"));
        ToolBox->addItem(page1, QString::fromUtf8("Page 2"));

        retranslateUi(ToolBox);

        ToolBox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ToolBox);
    } // setupUi

    void retranslateUi(QToolBox *ToolBox)
    {
        ToolBox->setWindowTitle(QApplication::translate("ToolBox", "ToolBox", 0, QApplication::UnicodeUTF8));
        ToolBox->setItemText(ToolBox->indexOf(page), QApplication::translate("ToolBox", "Page 1", 0, QApplication::UnicodeUTF8));
        ToolBox->setItemText(ToolBox->indexOf(page1), QApplication::translate("ToolBox", "Page 2", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(ToolBox);
    } // retranslateUi

};

namespace Ui {
    class ToolBox: public Ui_ToolBox {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TOOLBOX_H
