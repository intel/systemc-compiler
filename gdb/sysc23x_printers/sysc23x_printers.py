#!/usr/bin/python3
#
# Copyright (c) [2022] [Leonid Azarenkov, Intel]
#
import gdb


# Enable hex output format for sc_int/sc_uint
ScIntHexOut = 1

# Enable hex output format for sc_bigint/sc_biguint
ScBigIntHexOut = 1

# Enable hex output format for sc_bv/sc_lv
ScBvLvHexOut = 1


# Convert int val to string, for bool return 1/0
def striv(val):
    strv = str(val)
    if strv == "true":
        return "1"
    elif strv == "false":
        return "0"
    else:
        return strv


class ScSignal:
    """Print a sc_signal<> object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        iface = self.val
        curv = striv(iface.cast(iface.dynamic_type)["m_cur_val"])
        newv = striv(iface.cast(iface.dynamic_type)["m_new_val"])
        return newv if (curv == newv) else ("%s -> %s" % (curv, newv))

    def display_hint(self):
        return "sc_signal"


class ScSigPort:
    """Print a sc_in<>/sc_out<>/sc_inout<> object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        iface = self.val["m_interface"]
        return str(iface.cast(iface.dynamic_type).dereference())

    def display_hint(self):
        return "sc_port"


class ScInt_ScUInt:
    """Print a sc_int<>/sc_uint<> object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        if ScIntHexOut:
            return hex(self.val["m_val"]).upper().replace('X', 'x')
        else:
            return str(self.val["m_val"])

    def display_hint(self):
        return "sc_int"


class ScBigInt_ScBigUInt:
    """Print a sc_bigint<>/sc_biguint<> object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        BITS_PER_DIGIT = 30
        digmask = int((1 << BITS_PER_DIGIT) - 1)
        ndigits = self.val["ndigits"]
        sgn = int(self.val["sgn"])
        nbits = int(self.val["nbits"])

        intval = 0
        for i in reversed(range(0, ndigits)):
            digit = int(self.val["digit"][i])
            intval = (intval << BITS_PER_DIGIT) + (digmask & digit)

        if sgn == -1:
            intval = -intval

        if ScBigIntHexOut:
            return hex(intval).upper().replace('X', 'x')
        else:
            return str(intval)

    def display_hint(self):
        return "sc_bigint"


class ScBit:
    """Print a sc_bit object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "1" if str(self.val["m_val"]) == "true" else "0"

    def display_hint(self):
        return "sc_bit"


class ScBv:
    """Print a sc_bv<> object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        SC_DIGIT_SIZE = 32
        HBits = 4
        length = int(self.val["m_len"])
        size = int(self.val["m_size"])

        intval = 0
        for i in reversed(range(0, size)):
            digit = int(self.val["m_data"][i])
            intval = (intval << SC_DIGIT_SIZE) + digit

        if ScBvLvHexOut:
            hexstr = hex(intval)[2:].upper()
            padlen = int((length + HBits - 1) / HBits)
            hexstr = "0x" + hexstr.zfill(padlen)
            return hexstr
        else:
            binstr = "0b" + bin(intval)[2:].zfill(length)
            return binstr

    def display_hint(self):
        return "sc_bv"


# convert logic value to string
def strlv(val):
    if val == 0:
        return "0"
    elif val == 1:
        return "1"
    elif val == 2:
        return "Z"
    elif val == 3:
        return "X"
    else:
        return "U"


class ScLogic:
    """Print a sc_logic object."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return strlv(int(self.val["m_val"]))

    def display_hint(self):
        return "sc_logic"


class ScLV:
    """Print a sc_lv<> object."""

    def __init__(self, val):
        self.val = val

    def lvstr_to_hex(self, lvstr):
        hexstr = "0x"
        HBits = 4
        padlen = int((len(lvstr) + HBits - 1) / HBits) * HBits
        lvstr = lvstr.zfill(padlen)
        digits = [lvstr[i:i + HBits] for i in range(0, len(lvstr), HBits)]
        for dig in digits:
            if dig == "ZZZZ":
                hexstr += "Z"
            else:
                try:
                    hexstr += hex(int(dig, 2))[2:].upper()
                except ValueError:
                    hexstr += "X"
        return hexstr

    def to_string(self):
        SC_DIGIT_SIZE = 32
        lvstr = ""
        length = int(self.val["m_len"])
        size = int(self.val["m_size"])
        lst_digsz = length % SC_DIGIT_SIZE
        if lst_digsz == 0:
            lst_digsz = SC_DIGIT_SIZE

        for i in reversed(range(0, size)):
            data = int(self.val["m_data"][i])
            ctrl = int(self.val["m_ctrl"][i])

            digsz = lst_digsz if (i == size - 1) else SC_DIGIT_SIZE

            for j in reversed(range(0, digsz)):
                logic = 0
                val = 1 << j
                if (val & data) == val:
                    logic |= 1
                if (val & ctrl) == val:
                    logic |= 2
                lv = strlv(logic)
                lvstr += lv

        if ScBvLvHexOut:
            return self.lvstr_to_hex(lvstr)
        else:
            return "0b" + lvstr

    def display_hint(self):
        return "sc_lv"


class ScVector:
    """Print a sc_vector<> object."""

    class _iterator:
        def __init__(self, start, finish, itype):
            self.item = start
            self.finish = finish
            self.count = 0
            self.itype = itype
            self.vptr_type = gdb.lookup_type("void").pointer()

        def __iter__(self):
            return self

        def __next__(self):
            count = self.count
            self.count = self.count + 1
            if self.item == self.finish:
                raise StopIteration
            try:
                pelt = self.item.dereference().cast(self.vptr_type)
                elt = pelt.cast(self.itype.pointer()).dereference()
            except Exception:
                elt = "X"
            self.item = self.item + 1
            return ("[%d]" % count, elt)

    def __init__(self, val):
        itype = val.type.template_argument(0)
        self.typename = str(itype)
        self.itype = itype
        self.val = val

    def children(self):
        ovec = self.val["vec_"]
        return self._iterator(ovec["_M_impl"]["_M_start"],
                              ovec["_M_impl"]["_M_finish"],
                              self.itype)

    def to_string(self):
        ovec = self.val["vec_"]
        start = ovec["_M_impl"]["_M_start"]
        finish = ovec["_M_impl"]["_M_finish"]
        return ("%s of length %d"
                % (self.typename, int(finish - start)))

    def display_hint(self):
        return "array"


def build_pretty_printer():
    sysc23x_pp = gdb.printing.RegexpCollectionPrettyPrinter("SysC-23X")

    sysc23x_pp.add_printer("sc_bit", "^sc_dt::sc_bit$", ScBit)
    sysc23x_pp.add_printer("sc_bv", "^sc_dt::sc_bv<(.*)>$", ScBv)
    sysc23x_pp.add_printer("sc_logic", "^sc_dt::sc_logic$", ScLogic)
    sysc23x_pp.add_printer("sc_lv", "^sc_dt::sc_lv<(.*)>$", ScLV)
    sysc23x_pp.add_printer("sc_int", "^sc_dt::sc_int<(.*)>$", ScInt_ScUInt)
    sysc23x_pp.add_printer("sc_uint", "^sc_dt::sc_uint<(.*)>$", ScInt_ScUInt)
    sysc23x_pp.add_printer("sc_bigint", "^sc_dt::sc_bigint<(.*)>$", ScBigInt_ScBigUInt)
    sysc23x_pp.add_printer("sc_biguint", "^sc_dt::sc_biguint<(.*)>$", ScBigInt_ScBigUInt)
    sysc23x_pp.add_printer("sc_signed", "^sc_dt::sc_signed$", ScBigInt_ScBigUInt)
    sysc23x_pp.add_printer("sc_unsigned", "^sc_dt::sc_unsigned$", ScBigInt_ScBigUInt)
    sysc23x_pp.add_printer("sc_vector", "^sc_core::sc_vector<(.*)>$", ScVector)
    sysc23x_pp.add_printer("sc_signal", "^sc_core::sc_signal<(.*)>$", ScSignal)
    sysc23x_pp.add_printer("sc_in", "^sc_core::sc_in<(.*)>$", ScSigPort)
    sysc23x_pp.add_printer("sc_out", "^sc_core::sc_out<(.*)>$", ScSigPort)
    sysc23x_pp.add_printer("sc_inout", "^sc_core::sc_inout<(.*)>$", ScSigPort)

    return sysc23x_pp


def register_sysc23x_printers(val):
    gdb.printing.register_pretty_printer(
        gdb.current_objfile(),
        build_pretty_printer()
    )
