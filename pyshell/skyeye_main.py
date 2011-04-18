#! /usr/bin/env python

import cmd, sys, os
import string
import getopt
from ctypes import *
import skyeye_lib_path

_DEBUG=False


#main func#
if __name__ == '__main__':

    # Set library path through binary path
    path = sys.argv[0]
    skyeye_lib_path.set_skyeye_lib_path(path)

    # Check if arg valid
    opts = ""
    try:
        opts,args=getopt.getopt(sys.argv[1:],'be:dc:p:nl:hu')
    except getopt.GetoptError, err:
        print "**************************************"
        print "SkyEye args error!"
        print err
        print "Please get help through '-h' option"
        print "**************************************"
        exit()

    # Setup the debug mode
    if _DEBUG == True:
        import pdb
        pdb.set_trace()

    # Init skyeye opt
    from skyeye_init import init_opt
    init_opt(opts)

    #Skyeye cmdline
    from skyeye_common_module import *
    libcommon.get_autoboot.restype = c_uint
    autoboot = libcommon.get_autoboot()
    if autoboot == False:
        from skyeye_cli import *
        SkyEyeCli().cmdloop()
    else:
        print "\n***************"
        print "No interactive."
        print "***************\n"
        while 1:
            pass

#End main func#
