"""Generic Debugger Interface.
"""
from signal import SIGTRAP

from run import createChild
#from mm  import MemoryMaps
from ptrace.debugger import PtraceDebugger
from ptrace.debugger import ProcessExit, ProcessSignal, ProcessEvent
from ptrace.error import PtraceError
#from ptrace.ctypes_tools import (truncateWord, formatWordHex, formatAddress,
#                                 formatAddressRange, word2bytes)

ProcessExit = ProcessExit
ProcessSignal = ProcessSignal

class ProcessEnd(ProcessEvent):
  pass



class Debugger(PtraceDebugger):
    pass


class ProcessControl(object):
    def __init__(self):
        self.dbg = Debugger()
        self.process = None
        self.last_signal = []

    def start_process(self, binary, args, ea_start, ea_end):
        self.binary = binary
        self.filename = binary.filename
        self.args = list(args)
        self.ea_start = ea_start
        self.ea_end   = ea_end

        pid = createChild([self.filename]+self.args, 0)
        self.process = self.dbg.addProcess(pid, is_attached=1)

        #if end_addr:
        #    self.breakpoint(end_addr)

        if ea_start:
            self.breakpoint(ea_start)
            self.cont()

        return self.process

    def wait_event(self):
        event = self.dbg.waitProcessEvent()

        if not isinstance(event, ProcessExit):
            if self.process.getInstrPointer() == self.ea_end:
                return ProcessEnd(self.process, "Last instruction reached")

        return event

    """
    def get_process(self):
        return self.process

    """
    def detect_addr(self, ip):
       # Hit breakpoint?
       #ip = self.process.getInstrPointer()
       print hex(ip)
       breakpoint = self.process.findBreakpoint(ip-1)
       breakpoint.desinstall(set_ip=True)
       #self.mm = MemoryMaps(self.binary.filename, self.process.pid)
       #print self.mm
       #assert(0)


    def _continue_process(self, process, signum=None):
        if not signum and process in self.last_signal:
            signum = self.last_signal[process]

        if signum:
            error("Send %s to %s" % (signalName(signum), process))
            process.cont(signum)
            try:
                del self.last_signal[process]
            except KeyError:
                pass
        else:
            process.cont()

    def cont(self, signum=None):

        process = self.process
        process.syscall_state.clear()
        if process == self.process:
            self._continue_process(process, signum)
        else:
            self._continueProcess(process)

        signal = self.dbg.waitSignals()
        if signal.signum == SIGTRAP:
            self.detect_addr(process.getInstrPointer())


    def breakpoint(self, address):

        process = self.process
        # Create breakpoint
        size = None
        try:
            bp = process.createBreakpoint(address, size)
        except PtraceError, err:
            return "Unable to set breakpoint at %s: %s" % (
                formatAddress(address), err)
        #error("New breakpoint: %s" % bp)
        return bp

