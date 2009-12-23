typedef struct _sky_module_s{
}_sky_module_t;
class SkyModuleManger{
private:
	_sky_module_t* module;
public:
	void register_module();
	static sky_module_intf* getModuleManager();
	
}
SkyModule::register_module(){
}
SkyModule::unregister_module(){
}

