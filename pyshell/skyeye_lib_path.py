#### SkyEye lib path ####

skyeye_lib_path = ""

def set_skyeye_lib_path(path):
    global skyeye_lib_path
    path = path[:path.rfind('/') + 1]
    skyeye_lib_path = path + "../lib/skyeye/"
## End set_skyeye_lib_path ##

