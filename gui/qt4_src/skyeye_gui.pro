# -------------------------------------------------
# Project created by QtCreator 2009-03-15T20:45:16
# -------------------------------------------------
QT += network \
    xml \
    testlib
TARGET = skyeye-gui
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    console.cpp \
    mainmenu.cpp \
    machconf.cpp \
    skymain.cpp \
    devtreeview.cpp \
    machine.cpp \
    board.cpp \
    elfloader.cpp
HEADERS += mainwindow.h \
    console.h \
    mainmenu.h \
    machconf.h \
    skymain.h \
    devtreeview.h \
    machine.h \
    board.h \
    elfloader.h

INCPATH +=  ../../common/include/

LFLAGS	+= -rdynamic -Wl,-rpath,/opt/skyeye/lib/skyeye

LIBS	+= -rdynamic -Wl,-rpath,/opt/skyeye/lib/skyeye -L/opt/skyeye/lib/skyeye -lcommon -L/opt/skyeye/lib -lbfd -liberty 

