#### SkyEye common module ####
from ctypes import *
import skyeye_lib_path


libcommon_path = skyeye_lib_path.skyeye_lib_path + "/libcommon.so"

libcommon = CDLL(libcommon_path, RTLD_GLOBAL)
