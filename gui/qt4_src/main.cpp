#include <QtGui/QApplication>
#include <dlfcn.h>
#include "mainwindow.h"
#include "console.h"
#include "skymain.h"

#include "skyeye_types.h"
#include "skyeye_pref.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /*
     * libcommon will provide all the necessary API for simulator. we will
     * use the API in libcommon.so to communicate with SkyEye.
     */
    //void* common_handle = dlopen ("libcommon.so", RTLD_LAZY|RTLD_GLOBAL);
    //start_machine_func = dlsym(common_handler,"start_machine");
    /*
     * set some perference before start virtual machine.
     */
    sky_pref_t* pref = get_skyeye_pref();
    pref->interactive_mode = False;

    MainWindow* w = SkyMain::getMainWin();
    w->show();

    return a.exec();
}
