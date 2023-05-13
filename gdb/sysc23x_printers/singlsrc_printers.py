#!/usr/bin/python3
#
# Copyright (c) [2022] [Leonid Azarenkov, Intel]
#
import gdb


def striv(val):
    """ Convert int val to string, for bool return 1/0"""
    strv = str(val)
    if strv == "true":
        return "1"
    elif strv == "false":
        return "0"
    else:
        return strv


class SctSignalRtl:
    """Print a sct_signal<> object, RTL mode."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        iface = self.val
        curv = striv(iface.cast(iface.dynamic_type)["m_cur_val"])
        newv = striv(iface.cast(iface.dynamic_type)["m_new_val"])
        return newv if (curv == newv) else ("%s -> %s" % (curv, newv))

    def display_hint(self):
        return "sct_signal"


class SctSignalTlm:
    """Print a sct_signal<> object, TLM mode."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        curv = striv(self.val["curr_val"])
        newv = striv(self.val["next_val"])
        return newv if (curv == newv) else ("%s -> %s" % (curv, newv))

    def display_hint(self):
        return "sct_signal"


class SctSigPortRtl:
    """Print a sct_in<>/sct_out<> object, RTL mode"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        iface = self.val["m_interface"]
        return str(iface.cast(iface.dynamic_type).dereference())

    def display_hint(self):
        return "sct_port"


class SctSigPortTlm:
    """Print a sct_in<>/sct_out<> object, TLM mode."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        signal_type = gdb.lookup_type(str(self.val.type) + "::signal_type")
        iface = self.val["interface"]
        return str(iface.cast(signal_type.pointer()).dereference())

    def display_hint(self):
        return "sct_port"


class SctRegisterRtl:
    """Print a sct_register<> object, RTL mode."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        curv = striv(self.val["reg_data_d"]["m_cur_val"])
        newv = striv(self.val["reg_data"]["m_new_val"])
        return newv if (curv == newv) else ("%s => %s" % (curv, newv))

    def display_hint(self):
        return "sct_register"


class SctRegisterTlm:
    """Print a sct_register<> object, TLM mode."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        reg = self.val["reg"]
        curv = striv(reg["curr_val"]["m_cur_val"])
        newv = striv(reg["nextVal"])
        return newv if (curv == newv) else ("%s => %s" % (curv, newv))

    def display_hint(self):
        return "sct_register"


class SctFifoRtl:
    """Print a sct_fifo<> object, RTL mode."""

    def __init__(self, val):
        self.val = val
        self.empty_val = ""

    def children(self):
        result = []
        cthread_put = int(self.val["cthread_put"])
        cthread_get = int(self.val["cthread_get"])

        put_req_cur = int(self.val["put_req"]["m_cur_val"])
        put_req_nxt = int(self.val["put_req"]["m_new_val"])
        put_req_d_cur = int(self.val["put_req_d"]["m_cur_val"])

        get_req_cur = int(self.val["get_req"]["m_cur_val"])
        get_req_nxt = int(self.val["get_req"]["m_new_val"])
        get_req_d_cur = int(self.val["get_req_d"]["m_cur_val"])

        if cthread_put:
            has_put_cur = int(put_req_cur != put_req_d_cur)
            has_put_nxt = int(put_req_cur != put_req_nxt)
            if has_put_cur == has_put_nxt:
                has_put_str = "has_put: %d" % has_put_cur
            else:
                has_put_str = "has_put: %d -> %d" % (has_put_cur, has_put_nxt)
        else:
            has_put = str(self.val["put_req"])
            has_put_str = "has_put: %s" % has_put

        if cthread_get:
            has_get_cur = int(get_req_cur != get_req_d_cur)
            has_get_nxt = int(get_req_cur != get_req_nxt)
            if has_get_cur == has_get_nxt:
                has_get_str = "has_get: %d" % has_get_cur
            else:
                has_get_str = "has_get: %d -> %d" % (has_get_cur, has_get_nxt)
        else:
            has_get = str(self.val["get_req"])
            has_get_str = "has_get: %s" % has_get

        data_in = self.val["data_in"]
        data_out = self.val["data_out"]
        element_num = self.val["element_num"]
        fifo_size = int(self.val.type.template_argument(1))
        fifo_syn_val = int(self.val['SYNC_VALID'])
        fifo_syn_rdy = int(self.val['SYNC_READY'])
        params_str = "Size=%d, SyncVal=%d, SyncRdy=%d" % (fifo_size, fifo_syn_val, fifo_syn_rdy)

        result.append(("ready", self.val["ready_push"]))
        result.append(("data_in", data_in))
        result.append((has_put_str, ""))
        result.append(("request", self.val["out_valid"]))
        result.append(("data_out", data_out))
        result.append((has_get_str, ""))
        result.append(("element_num", element_num))
        result.append((params_str, ""))
        
        return result


def to_string(self):
    return str(self.val.type)


def display_hint(self):
    return "sct_fifo"


class SctInitiatorRtl:
    """Print a sct_initiator<> object, RTL mode."""

    def __init__(self, val):
        self.val = val

    def children(self):
        result = []
        sync = int(self.val["sync"])
        cthread = int(self.val["cthread"])
        always_ready = int(self.val["always_ready"])

        ready = 1 if always_ready else self.val["core_ready"]
        data = self.val["sync_data"] if sync else self.val["core_data"]
        if cthread:
            if sync:
                put_req_cur = int(self.val["sync_req"]["m_cur_val"])
                put_req_nxt = int(self.val["sync_req"]["m_new_val"])
                put_req_d_cur = int(self.val["sync_req_d"]["m_cur_val"])
            else:
                put_req_cur = int(self.val["put_req"]["m_cur_val"])
                put_req_nxt = int(self.val["put_req"]["m_new_val"])
                put_req_d_cur = int(self.val["put_req_d"]["m_cur_val"])

            has_put_cur = int(put_req_cur != put_req_d_cur)
            has_put_nxt = int(put_req_cur != put_req_nxt)
            if has_put_cur == has_put_nxt:
                has_put_str = "init.has_put: %d" % has_put_cur
            else:
                has_put_str = "init.has_put: %d -> %d" % (has_put_cur, has_put_nxt)
        else:
            has_put = str(self.val["sync_req"]) if sync else str(self.val["put_req"])
            has_put_str = "init.has_put: %s" % has_put

        params_str = "AlwaysReady=%d, Sync=%d" % (always_ready, sync)

        result.append(("init.ready", ready))
        result.append(("init.data", data))
        result.append((has_put_str, ""))
        result.append(("core_ready", self.val["core_ready"]))
        result.append(("core_req", self.val["core_req"]))
        result.append(("core_data", self.val["core_data"]))
        result.append((params_str, ""))

        return result

    def to_string(self):
        return str(self.val.type)

    def display_hint(self):
        return "sct_initiator"


class SctTargetRtl:
    """Print a sct_target<> object, RTL mode."""

    def __init__(self, val):
        self.val = val

    def children(self):
        result = []
        sync = int(self.val["sync"])
        cthread = int(self.val["cthread"])
        always_ready = int(self.val["always_ready"])
        has_fifo = int(self.val["fifo"])

        if has_fifo:
            fifo = self.val["fifo"]
            fifo = fifo.cast(fifo.dynamic_type)
            fifo_sz = int(fifo.type.template_argument(1))
        else:
            fifo = None
            fifo_sz = 0

        if has_fifo:
            request = fifo["out_valid"]
            data = fifo["data_out"]
        else:
            if always_ready:
                request = self.val["core_req_d"] if sync else self.val["core_req"]
                data = self.val["core_data_d"] if sync else self.val["core_data"]
            else:
                core_req = self.val["core_req"]["m_interface"]
                core_req = core_req.cast(core_req.dynamic_type).dereference()
                request = int(core_req["m_cur_val"]) or int(self.val["reg_full"]["m_cur_val"])
                sdata = sync or int(self.val["reg_full"]["m_cur_val"])
                data = self.val["core_data_d"] if sdata else self.val["core_data"]

        targ = fifo if has_fifo else self.val
        if cthread:
            get_req_cur = int(targ["get_req"]["m_cur_val"])
            get_req_nxt = int(targ["get_req"]["m_new_val"])
            get_req_d_cur = int(targ["get_req_d"]["m_cur_val"])

            has_get_cur = int(get_req_cur != get_req_d_cur)
            has_get_nxt = int(get_req_cur != get_req_nxt)
            if has_get_cur == has_get_nxt:
                has_get_str = "trgt.has_get: %d" % has_get_cur
            else:
                has_get_str = "trgt.has_get: %d -> %d" % (has_get_cur, has_get_nxt)
        else:
            has_get = str(targ["get_req"])
            has_get_str = "trgt.has_get: %s" % has_get

        params_str = "AlwaysReady=%d, Sync=%d, Fifo=%d" % (always_ready, sync, fifo_sz)

        result.append(("trgt.request", request))
        result.append(("trgt.data", data))
        result.append((has_get_str, ""))
        result.append(("core_ready", self.val["core_ready"]))
        result.append(("core_req", self.val["core_req"]))
        result.append(("core_data", self.val["core_data"]))
        result.append((params_str, ""))

        return result

    def to_string(self):
        return str(self.val.type)

    def display_hint(self):
        return "sct_target"


def build_pretty_printer():
    singlsrc_pp = gdb.printing.RegexpCollectionPrettyPrinter("SinglSrc")

    singlsrc_pp.add_printer("sct_comb_signal", "^sc_core::sct_comb_signal<(.*)>$", SctSignalRtl)
    
    singlsrc_pp.add_printer("sct_signal-rtl", "^sct::sct_signal<(.*), false>$", SctSignalRtl)
    singlsrc_pp.add_printer("sct_in-rtl", "^sct::sct_in<(.*), false>$", SctSigPortRtl)
    singlsrc_pp.add_printer("sct_out-rtl", "^sct::sct_out<(.*), false>$", SctSigPortRtl)
    singlsrc_pp.add_printer("sct_register-rtl", "^sct::sct_register<(.*), false>$", SctRegisterRtl)

    singlsrc_pp.add_printer("sct_signal-tlm", "^sct::sct_signal<(.*), true>$", SctSignalTlm)
    singlsrc_pp.add_printer("sct_in-tlm", "^sct::sct_in<(.*), true>$", SctSigPortTlm)
    singlsrc_pp.add_printer("sct_out-tlm", "^sct::sct_out<(.*), true>$", SctSigPortTlm)
    singlsrc_pp.add_printer("sct_register-tlm", "^sct::sct_register<(.*), true>$", SctRegisterTlm)

    singlsrc_pp.add_printer("sct_fifo-rtl", "^sct::sct_fifo<(.*), false>$", SctFifoRtl)
    singlsrc_pp.add_printer("sct_initiator-rtl", "^sct::sct_initiator<(.*), false>$", SctInitiatorRtl)
    singlsrc_pp.add_printer("sct_target-rtl", "^sct::sct_target<(.*), false>$", SctTargetRtl)

    return singlsrc_pp


def register_singlsrc_printers(val):
    gdb.printing.register_pretty_printer(
        gdb.current_objfile(),
        build_pretty_printer()
    )
