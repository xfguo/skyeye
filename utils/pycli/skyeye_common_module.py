#### SkyEye common module ####
import os
from ctypes import *

libcommon_path = os.getenv("SKYEYEBIN") + "/../lib/skyeye/libcommon.so"

libcommon = CDLL(libcommon_path, RTLD_GLOBAL)

#### SkyEye other modules ####
libdisasm_path = os.getenv("SKYEYEBIN") + "/../lib/skyeye/libdisasm.so"
libpmon_path = os.getenv("SKYEYEBIN") + "/../lib/skyeye/libpmon.so"
libuart_16550_path = os.getenv("SKYEYEBIN") + "/../lib/skyeye/libuart_16550.so"
libcodecov_path = os.getenv("SKYEYEBIN") + "/../lib/skyeye/libcodecov.so"
libbus_log_path = os.getenv("SKYEYEBIN") + "/../lib/skyeye/libbus_log.so"
libgdbserver_path = os.getenv("SKYEYEBIN") + "/../lib/skyeye/libgdbserver.so"

# Generate CDLL handlers
libdisasm = CDLL(libdisasm_path, RTLD_LOCAL)
libpmon = CDLL(libpmon_path, RTLD_LOCAL)
libuart_16550 = CDLL(libuart_16550_path, RTLD_LOCAL)
libcodecov = CDLL(libcodecov_path, RTLD_LOCAL)
libbus_log = CDLL(libbus_log_path, RTLD_LOCAL)
libgdbserver = CDLL(libgdbserver_path, RTLD_LOCAL)
