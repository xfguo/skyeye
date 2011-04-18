import cmd, sys
import skyeye_lib_path
from skyeye_module import *
from skyeye_common_module import *

def help_info_open():
    print "help info:"
    print "==========================================================="

def help_info_close():
    print "==========================================================="

class SkyEyeCli(cmd.Cmd):


################Start cli init #############################

    # Set the skyeye prompt
    prompt = "(skyeye) "

####### Check status to change prompt ########
    def postcmd(self, stop, line):
        if libcommon.SIM_is_running() == True:
            self.prompt = "(running) "
        else:
            self.prompt = "(skyeye) "
        return stop



    def default(self, line):
        self.stdout.write('*** Unknown command: %s\n'%line)
        self.stdout.write("*** Get commands with 'help' cmd\n")

    file = None

    def do_help(self, arg):
        if arg:
            # XXX check arg syntax
            try:
                func = getattr(self, 'help_' + arg)
            except AttributeError:
                try:
                    doc=getattr(self, 'do_' + arg).__doc__
                    if doc:
                        self.stdout.write("%s\n"%str(doc))
                        return
                except AttributeError:
                    pass
                self.stdout.write("%s\n"%str(self.nohelp % (arg,)))
                return
            func()
        else:
            names = self.get_names()
            cmds_doc = []
            cmds_undoc = []
            help = {}
            for name in names:
                if name[:5] == 'help_':
                    help[name[5:]]=1
            names.sort()
            # There can be duplicates if routines overridden
            prevname = ''
            for name in names:
                if name[:3] == 'do_':
                    if name == prevname:
                        continue
                    prevname = name
                    cmd=name[3:]
                    cmd = cmd.replace("_", "-")
                    if cmd in help:
                        cmds_doc.append(cmd)
                        del help[cmd]
                    elif getattr(self, name).__doc__:
                        cmds_doc.append(cmd)
                    else:
                        cmds_undoc.append(cmd)
#            self.stdout.write("%s\n"%str(self.doc_leader))
#            self.print_topics(self.doc_header,   cmds_doc,   15,80)
#            self.print_topics(self.misc_header,  help.keys(),15,80)
#            self.print_topics(self.undoc_header, cmds_undoc, 15,80)
            self.print_topics("\nSkyEye command list", cmds_undoc, 15,80)

    def complete(self, text, state):
        if state == 0:
            import readline
            origline = readline.get_line_buffer()
            line = origline.lstrip()
            stripped = len(origline) - len(line)
            begidx = readline.get_begidx() - stripped
            endidx = readline.get_endidx() - stripped
            if begidx>0:
                cmd, args, foo = self.parseline(line)
                if cmd == '':
                    compfunc = self.completedefault
                else:
                    try:
                        compfunc = getattr(self, 'complete_' + cmd)
                    except AttributeError:
                        compfunc = self.completedefault
            else:
                compfunc = self.completenames
            self.completion_matches = compfunc(text, line, begidx, endidx)
        try:
            return self.completion_matches[state]
        except IndexError:
            return None


    def completenames(self, text, *ignored):
        dotext = 'do_'+text
        string = [a[3:] for a in self.get_names() if a.startswith(dotext)]
        for index in range(len(string)):
            string[index] = string[index].replace("_", "-")

        return string

    def onecmd(self, line):
        line = line.replace("-", "_")
        cmd, arg, line = self.parseline(line)
        if not line:
            return self.emptyline()
        if cmd is None:
            return self.default(line)
        self.lastcmd = line
        if cmd == '':
            return self.default(line)
        else:
            try:
                func = getattr(self, 'do_' + cmd)
            except AttributeError:
                return self.default(line)
            return func(arg)

################ End cli init #############################





################ BEGIN COMMANDS DEFS#############################


####list-module list-options, list-machines, list-bp ####
    def do_list_modules(self, arg):
#        print "In do_list_modules"
        libcommon.com_list_modules(arg)

    def do_list_options(self, arg):
#        print "In do_list_options"
        libcommon.com_list_options(arg)

    def do_list_machines(self, arg):
#        print "In do_list_machines"
        libcommon.com_list_machines(arg)

    def do_list_bp(self, arg):
#        print "In do_list_bp"
        libcommon.com_list_bp(arg)


    def help_list_modules(self):
        help_info_open()
        print "list-modules : List all the loaded module."
        help_info_close()

    def help_list_options(self):
        help_info_open()
        print "list-options : List all the available options for SkyEye."
        help_info_close()

    def help_list_machines(self):
        help_info_open()
        print "list-machines : List all the supported machines for SkyEye."
        help_info_close()

    def help_list_bp(self):
        help_info_open()
        print "list-bp : List all the breakpoint."
        help_info_close()

####list-module list-options, list-machines, list-bp ####


########## show-pref, show-map, show-step ###############

    def do_show_pref(self, arg):
#        print "In do_show_pref"
        libcommon.com_show_pref(arg)

    def do_show_map(self, arg):
#        print "In do_show_map"
        libcommon.com_show_map(arg)

    def do_show_step(self, arg):
#        print "In do_show_step"
        libcommon.com_show_step(arg)


    def help_show_pref(self):
        help_info_open()
        print "show-pref : Show the current preference for SkyEye."
        help_info_close()

    def help_show_map(self):
        help_info_open()
        print "show-map : Show the current memory map for the machine."
        help_info_close()

    def help_show_step(self):
        help_info_open()
        print "show-step : Show the steps of current processor."
        help_info_close()


########## show-pref, show-map, show-step ###############


############# cov-state, cov-off, cov-on ################

    def do_cov_state(self, arg):
#        print "In do_cov_state"
        libcodecov.cov_state_show(arg)

    def do_cov_off(self, arg):
#        print "In do_cov_off"
        libcodecov.cov_state_off(arg)

    def do_cov_on(self, arg):
#        print "In do_cov_on"
        libcodecov.cov_state_on(arg)

    def help_cov_state(self):
        help_info_open()
        print "cov-state : show code coverage state."
        help_info_close()

    def help_cov_off(self):
        help_info_open()
        print "cov-off : turn off code coverage switch."
        help_info_close()

    def help_cov_on(self):
        help_info_open()
        print "cov-on : turn on code coverage switch."
        help_info_close()


############# cov-state, cov-off, cov-on ################

################## load-conf ##########################

    def do_load_conf(self, arg):
#        print "In do_load_conf"
        libcommon.com_load_conf(arg)

    def help_load_conf(self):
        help_info_open()
        print "load-conf : load a config file and parse it for SkyEye."
        help_info_close()

################## load-conf ##########################

################### delete-bp ##########################

    def do_delete_bp(self, arg):
#        print "In do_delete_bp"
        libcommon.com_delete_bp(arg)

    def help_delete_bp(self):
        help_info_open()
        print "delete-bp : Delete some breakpoint."
        help_info_close()

################### delete-bp ##########################

#################### log-bus ##########################

    def do_log_bus(self, arg):
#        print "In do_log_bus"
        libbus_log.com_log_bus(arg)

    def help_log_bus(self):
        help_info_open()
        print "log-bus : record every bus access to log file."
        help_info_close()

#################### log-bus ##########################

#################### remote-gdb ########################

    def do_remote_gdb(self, arg):
#        print "In do_remote_gdb"
        libgdbserver.com_remote_gdb()

    def help_remote_gdb(self):
        help_info_open()
        print "remote-gdb : Open remote gdb debugger."
        help_info_close()

#################### remote-gdb ########################

#################### other COMMANDS ####################

    def do_ls(self, arg):
#        print "outline ls"
        libcommon.com_list(arg)

    def do_quit(self, arg):
#        print "outline quit"
        libcommon.com_quit(arg)

    do_q = do_quit

    def do_run(self, arg):
#        print "outline run"
        libcommon.com_run(arg)

    def do_stop(self, arg):
#        print "outline stop"
        libcommon.com_stop(arg)

    def do_continue(self, arg):
#        print "outline continue"
        libcommon.com_cont(arg)

    def do_stepi(self, arg):
#        print "outline stepi"
        libcommon.com_si(arg)

    def do_start(self, arg):
#        print "outline start"
        libcommon.com_start(arg)

    def do_info(self, arg):
#        print "outline info"
        libcommon.com_info(arg)

    def do_x(self, arg):
#        print "outline x"
        libcommon.com_x(arg)

    def do_pmon(self, arg):
#        print "outline pmon"
        libpmon.com_pmon(arg)

    def do_create_uart_16550(self, arg):
#        print "outline create_uart_16550"
        libuart_16550.com_create_16550

    def do_disassemble(self, arg):
#        print "outline disassemble"
        libdisasm.com_disassemble(arg)

    def do_break(self, arg):
#        print "outline do_break"
        libcommon.com_break(arg)


    def help_ls(self):
        help_info_open()
        print "ls : Synonym for `list'"
        help_info_close()

    def help_quit(self):
        help_info_open()
        print "quit : Quit SkyEye " 
        help_info_close()

    def help_q(self):
        help_info_open()
        print "q : Quit SkyEye " 
        help_info_close()


    def help_run(self):
        help_info_open()
        print "run : Start the simulator." 
        help_info_close()

    def help_stop(self):
        help_info_open()
        print "stop : Stop the running of simulator." 
        help_info_close()

    def help_continue(self):
        help_info_open()
        print "continue : Continue the running of interrupted simulator." 
        help_info_close()

    def help_stepi(self):
        help_info_open()
        print "stepi : step into ." 
        help_info_close()

    def help_start(self):
        help_info_open()
        print "start : start simulator." 
        help_info_close()

    def help_info(self):
        help_info_open()
        print "info : show information for various objects. "
        help_info_close()

    def help_x(self):
        help_info_open()
        print "x : display memory value at the address. "
        help_info_close()

    def help_pmon(self):
        help_info_open()
        print "pmon : enable the performance monitor." 
        help_info_close()

    def help_create_uart_16550(self):
        help_info_open()
        print "create_uart_16550 : Create a new uart of 16550 type."
        help_info_close()

    def help_disassemble(self):
        help_info_open()
        print "disassemble : Disassemble the given address."
        help_info_close()

    def help_break(self):
        help_info_open()
        print "break : disassemble : set breakpoint for an address."
        help_info_close()


#################### other COMMANDS ####################
##### End with Class SkyEye #####
