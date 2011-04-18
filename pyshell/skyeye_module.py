#### SkyEye module ####
from ctypes import *
import skyeye_lib_path

# Combine the libraries' absolute path
libdisasm_path = skyeye_lib_path.skyeye_lib_path + "/libdisasm.so"
libpmon_path = skyeye_lib_path.skyeye_lib_path + "/libpmon.so"
libuart_16550_path = skyeye_lib_path.skyeye_lib_path + "/libuart_16550.so"
libcodecov_path = skyeye_lib_path.skyeye_lib_path + "/libcodecov.so"
libbus_log_path = skyeye_lib_path.skyeye_lib_path + "/libbus_log.so"
libgdbserver_path = skyeye_lib_path.skyeye_lib_path + "/libgdbserver.so"

# Generate CDLL handlers
libdisasm = CDLL(libdisasm_path, RTLD_LOCAL)
libpmon = CDLL(libpmon_path, RTLD_LOCAL)
libuart_16550 = CDLL(libuart_16550_path, RTLD_LOCAL)
libcodecov = CDLL(libcodecov_path, RTLD_LOCAL)
libbus_log = CDLL(libbus_log_path, RTLD_LOCAL)
libgdbserver = CDLL(libgdbserver_path, RTLD_LOCAL)
