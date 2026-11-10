#include <sct_common.h>

using namespace sc_core;
using namespace sct;

struct CastSign : sc_module {
    sct_in<sct_int<11>> SC_NAMED(signed_in);
    sct_in<sct_uint<11>> SC_NAMED(unsigned_in);
    sct_signal<sct_int<11>> SC_NAMED(signed_signal);
    sc_vector<sct_out<unsigned long long>> SC_NAMED(warn_out, 18);
    sc_vector<sct_out<unsigned long long>> SC_NAMED(quiet_out, 22);

    SC_HAS_PROCESS(CastSign);

    explicit CastSign(sc_module_name name) : sc_module(name) {
        SC_METHOD(drive_signal);
        sensitive << signed_in;
        SC_METHOD(proc);
        sensitive << signed_in << unsigned_in << signed_signal;
    }

    void drive_signal() {
        signed_signal = signed_in.read();
    }

    unsigned long long pass(unsigned long long data) {
        return data;
    }

    unsigned long long castReturn(int data) {
        return static_cast<unsigned>(data);
    }

    void proc() {
        const sct_int<11> signed_value = signed_in.read();
        const sct_uint<11> unsigned_value = unsigned_in.read();
        const int signed_data = signed_value.to_int();
        const unsigned unsigned_data = unsigned_value.to_uint();

        warn_out[0] = (unsigned short)signed_data;
        warn_out[1] = unsigned(signed_data);
        warn_out[2] = static_cast<unsigned long long>(signed_data);
        warn_out[3] = (short)unsigned_data;
        warn_out[4] = int(unsigned_data);
        warn_out[5] = static_cast<long long>(unsigned_data);

        warn_out[6] = (sct_uint<10>)signed_value;
        warn_out[7] = sct_uint<11>(signed_value);
        warn_out[8] = static_cast<sct_uint<16>>(signed_value);
        warn_out[9] = (sct_int<10>)unsigned_value;
        warn_out[10] = sct_int<11>(unsigned_value);
        warn_out[11] = static_cast<sct_int<16>>(unsigned_value);
        warn_out[12] = (sct_uint<4>(0u), sct_uint<10>(signed_value));
        warn_out[13] = (static_cast<unsigned>(signed_data));
        warn_out[14] = static_cast<unsigned long long>(
            static_cast<unsigned>(signed_data));
        warn_out[15] = pass(static_cast<unsigned>(signed_data));
        warn_out[16] = signed_value.to_uint();
        warn_out[17] = signed_signal.read().to_uint();

        unsigned long long warn_init = static_cast<unsigned>(signed_data);
        unsigned long long warn_assign;
        warn_assign = static_cast<unsigned>(signed_data);
        unsigned long long warn_array[1] = {unsigned(signed_data)};

        quiet_out[0] = unsigned_data;
        quiet_out[1] = unsigned(unsigned_data);
        quiet_out[2] = int(signed_data);
        quiet_out[3] = sct_uint<16>(unsigned_value);
        quiet_out[4] = sct_int<16>(signed_value);
        quiet_out[5] = bool(signed_data);
        quiet_out[6] = unsigned(true);
        quiet_out[7] = (unsigned)5;
        quiet_out[8] = int(5u);
        quiet_out[9] = sct_uint<10>(5);
        quiet_out[10] = sct_int<10>(5u);
        quiet_out[11] = static_cast<unsigned>(5);
        quiet_out[12] = (unsigned)(-5);
        quiet_out[13] = warn_init;
        quiet_out[14] = warn_assign;
        quiet_out[15] = warn_array[0];
        quiet_out[16] = castReturn(signed_data);
        quiet_out[17] = static_cast<int>(unsigned_data) + 1;
        quiet_out[18] = sct_uint<10>(signed_value)[0];
        quiet_out[19] = unsigned_value.to_uint();
        quiet_out[20] = unsigned_in.read().to_uint();
        quiet_out[21] = sct_int<11>(5).to_uint();
        (void)static_cast<unsigned>(signed_data);
    }
};

int sc_main(int argc, char** argv) {
    sct_signal<sct_int<11>> SC_NAMED(signed_in);
    sct_signal<sct_uint<11>> SC_NAMED(unsigned_in);
    sc_vector<sct_signal<unsigned long long>> SC_NAMED(warn_out, 18);
    sc_vector<sct_signal<unsigned long long>> SC_NAMED(quiet_out, 22);
    CastSign SC_NAMED(top);
    top.signed_in(signed_in);
    top.unsigned_in(unsigned_in);
    top.warn_out(warn_out);
    top.quiet_out(quiet_out);
    sc_start(SC_ZERO_TIME);
    return 0;
}