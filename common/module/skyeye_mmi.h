/*
* ========== Copyright Header Begin ==========================================
* 
* OpenSPARC T1 Processor File: mmi.h
* Copyright (c) 2006 Sun Microsystems, Inc.  All Rights Reserved.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES.
* 
* The above named program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License version 2 as published by the Free Software Foundation.
* 
* The above named program is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
* 
* You should have received a copy of the GNU General Public
* License along with this work; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
* 
* ========== Copyright Header End ============================================
*/
/*
 * mmi.h
 * SAM device-model API
 */
#ifndef _SAM_MMI_H
#define _SAM_MMI_H

#pragma ident "@(#)1.26	03/12/05	SAM-mmi.h"

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


/* opaque type definitions for MMI device models */
typedef void * mmi_instance_t;
typedef void * mmi_module_t;

typedef enum {mmi_false=0, mmi_true=1} mmi_bool_t;

/* device operation in SAM
 *
 * 1. The simulator reads in the simulation configuration file containing "sysconf" lines at startup
 * 2. Each sysconf line specifies a module-type name, an instance name and a set of properties in the form name=value
 *   For example:
 *   sysconf mydev dev0 baseaddr=0xffc00040 parent=bus0 debug=true logfile=mydev.0.log
 * 3. The sysconf parser loads the mydev.so object (unless it is already loaded)
 * 4. The sysconf parser calls the instance creator function in mydev.so to create the "dev0" instance
 * 5. The dev0 instance creator retrieves its sysconf arguments using mmi_argc and mmi_argv calls and processes them
 * 6. The dev0 instance creator maps its device registers into the SAM address space using mmi_map_physio
 */

/* An MMI device model is a shared object loaded using dlopen().
 * In SAM, this mechanism requires an _init function in the shared object:
 *
 *   extern "C" void _init()
 *
 * This function must be defined. For example:
 *
 *   extern "C" void _init() {
 *     // mmi initialization code goes here
 *   }

 * There are alternative means of automatically invoking an initialization function in a shared object
 * For example, the constructor of a static object in the shared object written using C++
 *
 * In any event, SAM does not explicitly call a function in the shared object after dlopen().
 * The _init() function is implicitly invoked by dlopen().
 */

/* Functions returning mmi_bool_t return an SUCCESS status (true=no error).
 * An error typically indicates invalid arguments are being passed
 */

/* Devices are instantiated by SAM when specified in the config file.
 * For each device instance, the device model needs to provide an INSTANCE CREATOR function
 * which performs the instantiation.
 *
 * The instance creator function is invoked from SAM
 * with the modname (device class name) and the instance-name
 */

typedef void (*mmi_instance_creator) (const char *modname, const char *instance_name);

mmi_bool_t mmi_register_instance_creator(const char * modname, mmi_instance_creator creatorfn);

/* the instance_creator function can retrieve its own handle (mmi_instance_t) by calling the mmi_register_instance
 * function. In addition, the device-model can register it's implementation-specific instance data. This can be
 * retrieved (eg in the modinfo, config and interface callbacks) using the mmi_get_instance_data call.
 * The help string is displayed in response to UI commands for listing all device nodes.
 *
 * Putting the mmi_instance_t handle in the instance_data structure can be convenient
 */

mmi_instance_t mmi_register_instance(const char * modname, const char * instancename, void * instance_data, const char * short_help);

void * mmi_get_instance_data(mmi_instance_t instance);



/* Register a function that can respond to the modinfo UI command by printing out relevant module instance information */
typedef void (*mmi_modinfo_cb) (mmi_instance_t cb_instance);

mmi_bool_t mmi_register_modinfo_cb(mmi_instance_t this_instance, mmi_modinfo_cb modinfo_fn);



/* These calls use the mmi_instance_t handle provided by the mmi_register_instance function above.
 * They return the arguments specified on the sysconf line for this device instance
 */
int mmi_argc(mmi_instance_t this_instance);
char * mmi_argv(mmi_instance_t this_instance, int index);



/* A function of type mmi_access needs to be provided by the device model to handle accesses (load/store) to
 * device registers in response to instructions executed by the simulated CPU.
 *
 * The mmi_map_physio function is used to register this function with the simulator. The device-model provides a pointer
 * to user data which is returned in the call to the access function. This function should return an error indication (0 for success, some non-zero value for failure
 */
typedef int  (*mmi_access)	(uint32_t cpuid, void* obj, uint64_t paddr, mmi_bool_t wr, uint32_t size, uint64_t* buf, uint8_t bytemask);

/* These calls lets the instance map/unmap its registers onto the physical address space of the simulated CPUs */
int  mmi_map_physio  (uint64_t base, uint64_t size, void* obj, mmi_access access_fn);
void mmi_unmap_physio(uint64_t base, uint64_t size, void* obj);



/* Memory access functions for devices: devices can do block read/write into simulated memory using mmi_memread and mmi_memwrite calls */
void mmi_memread(uint64_t paddr, uint8_t * data, uint64_t size);
void mmi_memwrite(uint64_t paddr, const uint8_t * data, uint64_t size);


/* interrupts: interrupts are not directly delivered to CPUs by devices. Instead,
 * they are passed on up the device hierarchy through a bus and/or a bridge. The bridge model
 * implements interrupt delivery to the appropriate CPU. We do not address this in MMI
 */



/* start/stop functions
 * device operation can be synchronous (in response to a sparc-cpu action and within a cpu simulation thread)
 * or asynchronous (within a separate thread). A network controller thread listening on a socket for simulated
 * network connections is an example of an asynchronous device model.
 *
 * When a run or stop command is issued at the UI prompt, an asynchronous thread needs to start/stop in response
 * (eg start=>create thread; stop=>kill thread).
 * The start/stop functions provide a way for SAM to communicate these changes in run state to asynchronous devices
 *
 * These functions are not needed for synchronous devices (devices that do not create their own simulation threads)
 */
typedef void (*mmi_start_stop_cb) (void * userdata);
void mmi_register_start_stop(mmi_start_stop_cb start_action, mmi_start_stop_cb stop_action, void * userdata);


/* checkpointing functions
 * SAM has a checkpointing feature (dump/restore) which allows a user to dump current simulator state
 * or to restore (at init-time) from a previous checkpoint.
 * A module needs to register dump and restore functions with SAM to respond to dump/restore UI actions.
 * SAM calls the dump and restore functions with the directory in which the device instance should create/find
 * its state dump. The name of the dump file is <instancename>.dmp
 * These functions should return a success indication (true=success, false=failure)
 */
typedef mmi_bool_t (*mmi_dump_cb)(void * userdata, const char * dirname);
typedef mmi_bool_t (*mmi_restore_cb)(void * userdata, const char * dirname);

mmi_bool_t mmi_register_dump_restore( const char *name, mmi_dump_cb dump_fn, mmi_restore_cb restore_fn, void * userdata);



/* time-related functions
 * A device may want to model a delay or a periodic process (eg device latency or device internal clock)
 *
 * SAM and the MMI interface support a delayed event callback mechanism. The chosen unit of time is
 * microseconds of simulated time.
 *
 * The device model
 *   defines a callback function (void function with two void * parameters)
 *   gets the current time using the mmi_get_time() function.
 *   adds the desired delay to the current time
 *   registers a delayed callback using the mmi_register_event function
 *
 * For periodic processes, the event callback function should register itself at time=current+period
 */
typedef void (*mmi_event_cb) (void * userdata1, void * userdata2);
int64_t mmi_get_time(); // current simulated time in microseconds since reboot
mmi_bool_t mmi_register_event(int64_t when, mmi_event_cb event_fn, void * userdata1, void * userdata2);


/* Interacting with other devices */

/*
 * When a device is instantiated/deleted in response to a sysconf directive,
 * a device module can receive about this config change by registering a config callback
 * function.
 * 
 * A config event can be of the type: instance-added, instance-deleted and config-init-done
 * config-init-done refers to simulated system being initialized and ready to execute instructions.
 * instance_deleted is deprecated and obsolete. Simulator should not delete a device instance.

 * registering a config callback results in the callback function
 * being called for config events that have occurred in the past as
 * well as those that occur subsequently.
 */

typedef enum {MMI_CONFIG_NEW_MODULE, MMI_CONFIG_DELETE_MODULE, MMI_CONFIG_INIT_DONE} mmi_config_t;
typedef void (*mmi_config_cb)    (void *callback_data, mmi_instance_t target,  const char * target_name, mmi_config_t);
mmi_bool_t mmi_register_config_cb(mmi_instance_t this_instance, mmi_config_cb config_fn);

// get a device-instance handle by name
mmi_instance_t mmi_get_instance(const char * instancename);

// register a call-back function with the simulator to respond to mmi_get_interface calls from other modules
typedef void* (*mmi_interface_cb) (void *callback_data, const char *name);
mmi_bool_t mmi_register_interface_cb(mmi_instance_t this_instance, mmi_interface_cb);

// get a named interface from another device instance (eg pcie_device from pcie_bus).
// the return value should be typecast to the mutually-agreed-upon interface type
void * mmi_get_interface(mmi_instance_t instance, const char * interface_name);




/* for non-peripheral "devices" that implement ASIs */
/* register call back functions for asi load/store for a particular asi */
typedef int  (*mmi_ld_asi_action)	(void *cbd, uint32_t asi, uint64_t vaddr, uint64_t *buf, int size,uint32_t cpuid);
typedef int  (*mmi_st_asi_action)	(void *cbd, uint32_t asi, uint64_t vaddr, uint64_t buf, int size,uint32_t cpuid);
void mmi_register_asi_action (mmi_instance_t instance, uint32_t asi, mmi_ld_asi_action ld_handler, mmi_st_asi_action st_handler); 

/* register the call back data that will passed thro' the asi load/store callback function (eg: the object pointer that handles the asi) */
mmi_bool_t   mmi_register_asi_cb_data (mmi_instance_t instance, void *cb_data);


/* UI commands in MMI objects
 * A user interface command can be module-specific or instance-specific
 * A module writer can choose to implement module-specific or instance-specific commands (or both)
 * A module-specific command has the module name as its first word, while the instance-specific command
 * starts with the instance-name as its first word. For example, if the simulator is configured with 2
 * instances of a device called mydev:
 *
 *   sysconf mydev dev1 ...
 *   sysconf mydev dev2 ...
 *
 * Then a module-specific ui command might look like this:
 *
 *   mydev show-flags
 *
 * while an instance-specific ui command might look like this:
 *
 *  dev1 report-stats -all
 *
 * At instantiation, a module (or instance) registers its UI command handler with the simulation framework
 * A UI command is intercepted by the simulator, and the module/instance function corresponding to the
 * command is called.
 *
 * When the instance function is called, it is passed the
 * instance-data pointer that was provided by the instance creator to
 * the mmi_register_instance() function.
 *
 * The handler functions should return an error code (0 if no error).
 * NOTE: the error code is currently not read by SAM
 */

typedef int (*mmi_module_cmd_fn) (void * nullptr, int argc, char **argv);
void mmi_register_module_cmd(mmi_module_t mod, const char * helpstring, mmi_module_cmd_fn fn);

typedef int (*mmi_instance_cmd_fn) (void * instancedata, int argc, char **argv);
void mmi_register_instance_cmd(mmi_instance_t instance, const char * helpstring, mmi_instance_cmd_fn fn);




/* old mmi functions (for backwards compatibility - eg the SN sync-device model) */
mmi_bool_t mmi_unregister_instance_creator(mmi_instance_t instance);

/* these have been deprecated in favor of mmi_map_physio (see above) */


typedef int (*mmi_io_action) (void *cb_data, uint64_t paddr, uint64_t *buf, int size, uint8_t bytemask, void *cpuptr);
int mmi_register_io_action (mmi_module_t *module, mmi_io_action ld_handler, mmi_io_action st_handler); 


/* interrupt functions - these are described in the MMI documentation */
#if 0 // this wont work for io devices.!...
int mmi_interrupt_packet (int dest_cpuid, void *src, int src_iscpu, uint64_t *idata); 
#else
int mmi_interrupt_packet (int dst_aid, int src_aid, uint64_t *idata); 
#endif
int mmi_interrupt_vector (int dest_cpuid, void *src, int src_iscpu, uint32_t vnum, int traptype); 

typedef void (*mmi_event_cycle)(void * instance_data, uint64_t repeat);
void*      mmi_register_cb_cycle  (mmi_instance_t instance, mmi_event_cycle handler, uint64_t repeat);
void       mmi_unregister_cb_cycle(mmi_instance_t instance, void * intf);
int        mmi_disable_cb_cycle   (void * intf);
int        mmi_enable_cb_cycle    (void * intf, uint64_t repeat);


uint64_t mmi_get_cpufreq();

#if 0
{
#endif
#ifdef __cplusplus
} // extern "C"
#endif


#endif //  _SAM_MMI_H
