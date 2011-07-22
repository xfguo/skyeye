#### SkyEye common module ####
import os
from ctypes import *

libcommon_path = os.getenv("SKYEYEBIN") + "/../lib/skyeye/libcommon.so"

libcommon = CDLL(libcommon_path, RTLD_GLOBAL)
