from ctypes import *
import string
import struct
from skyeye_common_module import *


def set_exec_path(file_path, libcommon):
    libcommon.set_exec_file.argtypes = [c_char_p]
    libcommon.set_exec_file(file_path)
    libcommon.get_exec_file.restype = c_char_p
    exec_file = libcommon.get_exec_file()


PACKAGE_STRING = "SkyEye 1.3.3"
PACKAGE_BUGREPORT = "skyeye-simulator@googlegroups.com"

def usage():
    global PACKAGE_STRING, PACKAGE_BUGREPORT

    print  "%s" %PACKAGE_STRING
    print  "Bug report: %s" %PACKAGE_BUGREPORT

    print  "Usage: skyeye [options] -e program [program args]"
    print  "Default mode is STANDALONE mode"
    print  "------------------------------------------------------------------"
    print  "Options:"
    print  "-e exec-file        the  ELF executable format kernel file name."
    print  "-n         non-interactive mode, means we do not have available command line ."
    print  "-l load_address,load_address_mask"
    print  "                    Load ELF file to another address, not its entry."
    print  "-b                  specify the data type is big endian when non '-e' option."
    print  "-d                  in GDB Server mode  can be connected by GDB ."
    print  "-c config-file      the skyeye configure file name."
    print  "-h                  The SkyEye command options, and ARCHs and CPUs simulated."
    print  "------------------------------------------------------------------"
    exit()


def set_config_path(file_path, libcommon):
    libcommon.set_conf_filename.argtypes = [c_char_p]
    libcommon.set_conf_filename(file_path)
    libcommon.get_conf_filename.restype = c_char_p
    conf_file = libcommon.get_conf_filename()


def set_uart_port(opt, libcommon):
    libcommon.set_uart_port.argtypes = [c_int]
    libcommon.set_uart_port(int(opt, 16))
    libcommon.get_uart_port.restype = c_int
    uart_port_new = libcommon.get_uart_port()

def set_auto_mode(opt, libcommon):
    libcommon.set_interactive_mode(0)
    libcommon.get_interactive_mode.restype = c_uint
    mode = libcommon.get_interactive_mode()
    libcommon.set_autoboot(opt)
    libcommon.get_autoboot.restype = c_uint
    autoboot = libcommon.get_autoboot()


def set_load_conf(opt, libcommon):
    parts = opt.split(',')
    # exec_load_base
    libcommon.set_exec_load_base.argtypes = [c_uint]
    libcommon.set_exec_load_base(int(parts[0], 16))

    # exec_load_mask
    libcommon.set_exec_load_mask(int(parts[1], 16))
    libcommon.get_exec_load_mask.restype = c_uint
    load_mask = libcommon.get_exec_load_mask()

def set_endian(arg, libcommon):
    libcommon.set_endian.argtypes = [c_int]
    libcommon.set_endian(arg)
    libcommon.get_endian.restype = c_int
    endian = libcommon.get_endian()

def set_ctrl_mode(arg, libcommon):
    # ctrl_mode
    libcommon.set_user_mode.argtypes = [c_int]
    libcommon.set_user_mode(arg)
    libcommon.get_user_mode.restype = c_int
    endian = libcommon.get_user_mode()



LITTLE_ENDIAN = 0
BIG_ENDIAN = 1

NONE_AUTO_RUN = 0
AUTO_RUN = 1

SYS_MODE = 0
USR_MODE = 1


# Command line argument parse
def dispatch(type, cont, libcommon):
    global LITTLE_ENDIAN, BIG_ENDIAN
    global NONE_AUTO_RUN, AUTO_RUN
    global SYS_MODE, USR_MODE

    dispatchion = {'-e':lambda:set_exec_path(cont, libcommon),
                   '-h':lambda:usage(),
                   '-c':lambda:set_config_path(cont, libcommon),
                   '-p':lambda:set_uart_port(cont, libcommon),
                   '-n':lambda:set_auto_mode(AUTO_RUN, libcommon),
                   '-l':lambda:set_load_conf(cont, libcommon),
                   '-b':lambda:set_endian(BIG_ENDIAN, libcommon),
                   '-u':lambda:set_ctrl_mode(USR_MODE, libcommon)}
#                   '-?':lambda:set_config_path(cont)}
    dispatchion[type]()



# initialize the option of loading.

def init_opt(args):

    global LITTLE_ENDIAN, BIG_ENDIAN
    global NONE_AUTO_RUN, AUTO_RUN
    global SYS_MODE, USR_MODE

    # init skyeye_pref struction

    libcommon.get_skyeye_pref()
    set_config_path("./skyeye.conf", libcommon)
    set_load_conf("0x0,0xFFFFFFFF", libcommon)
    set_endian(LITTLE_ENDIAN, libcommon)
    set_ctrl_mode(SYS_MODE, libcommon)

    # Reinitialization with command line args
    for x in args:
        dispatch(x[0], x[1], libcommon)

    # Simulator intialize
    libcommon.SIM_init()
