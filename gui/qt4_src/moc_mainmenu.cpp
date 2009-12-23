/****************************************************************************
** Meta object code from reading C++ file 'mainmenu.h'
**
** Created: Sat Aug 22 22:26:44 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainmenu.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainmenu.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainMenu[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x08,
      18,    9,    9,    9, 0x08,
      29,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainMenu[] = {
    "MainMenu\0\0about()\0openConf()\0openElf()\0"
};

const QMetaObject MainMenu::staticMetaObject = {
    { &QMenuBar::staticMetaObject, qt_meta_stringdata_MainMenu,
      qt_meta_data_MainMenu, 0 }
};

const QMetaObject *MainMenu::metaObject() const
{
    return &staticMetaObject;
}

void *MainMenu::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainMenu))
	return static_cast<void*>(const_cast< MainMenu*>(this));
    return QMenuBar::qt_metacast(_clname);
}

int MainMenu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMenuBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: about(); break;
        case 1: openConf(); break;
        case 2: openElf(); break;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
