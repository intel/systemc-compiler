/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 11/8/18.
//

#include <systemc.h>


enum apb_resp { ERROR = 1, SUCCESS = 0 };

template <typename addr_t, typename data_t>
struct apb_if
{
    virtual void apb_reset() = 0;
    virtual apb_resp apb_trans(addr_t addr, data_t &data, bool is_read) = 0;
};

template <template<class> typename MPortT,
          template<class> typename SPortT,
          typename AddrT, typename DataT>
struct apb_signal_if
{
    MPortT<AddrT>  paddr{"paddr"};
    MPortT<bool>   psel{"psel"};
    MPortT<bool>   penable{"penable"};
    MPortT<bool>   pwrite{"pwrite"};
    MPortT<DataT>  pwdata{"pwdata"};
    SPortT<bool>   pready{"pready"};
    SPortT<DataT>  prdata{"prdata"};
    SPortT<bool>   pslverr{"pslverr"};
};

template <typename AddrT, typename DataT>
struct apb_slave_signal_if : apb_signal_if<sc_in, sc_out, AddrT, DataT> {};

struct sync_module : sc_module
{
    sc_in<bool> clk{"clk"};
    sc_in<bool> rstn{"rstn"};

protected:
    void before_end_of_elaboration() override
    {
        for(auto *child: get_child_objects()) {
            if (sync_module *sync_child = dynamic_cast<sync_module *>(child)) {
                sync_child->clk(clk);
                sync_child->rstn(rstn);
            }
        }
    }

#define SYNC_THREAD(func) \
    SC_CTHREAD(func, clk.pos()); \
    async_reset_signal_is(rstn, false);

};

template <typename AddrT, typename DataT>
struct apb_slave : sync_module, apb_slave_signal_if<AddrT, DataT>, sc_interface
{
    using apb_slave_signal_if<AddrT, DataT>::paddr;
    using apb_slave_signal_if<AddrT, DataT>::psel;
    using apb_slave_signal_if<AddrT, DataT>::penable;
    using apb_slave_signal_if<AddrT, DataT>::pwrite;
    using apb_slave_signal_if<AddrT, DataT>::pwdata;
    using apb_slave_signal_if<AddrT, DataT>::pready;
    using apb_slave_signal_if<AddrT, DataT>::prdata;
    using apb_slave_signal_if<AddrT, DataT>::pslverr;


    apb_slave(sc_module_name, apb_if<AddrT, DataT> * impl)
    : impl(impl)
    {
        SC_HAS_PROCESS(apb_slave);
        SYNC_THREAD(apb_thread);
    }

private:
    apb_if<AddrT, DataT> * impl;

    void apb_thread() {
        impl->apb_reset();
        pready = 0;
        prdata = 0;
        pslverr = 0;
        while (true) {
            wait();
            pready = 0;
            if (psel) {
                DataT data;
                if (pwrite) {
                    data = pwdata;
                    pslverr = impl->apb_trans(paddr, data, 0);
                } else {
                    pslverr = impl->apb_trans(paddr, data, 1);
                    prdata = data;
                }
                pready = 1;
                wait();
            }
            pready = 0;
        }
    }

};

template <typename AddrT, typename DataT, size_t REGFILE_SIZE>
struct dut : sync_module, apb_if<AddrT, DataT> {
    SC_CTOR(dut) {}
    apb_slave<AddrT, DataT> apb_tar{ "apb_tar", this };

private:

    DataT  reg_file[REGFILE_SIZE];

    void apb_reset() override {  }

    apb_resp apb_trans (AddrT addr, DataT &data, bool is_read) override
    {
        if (addr < REGFILE_SIZE) {
            if (is_read)
                data = reg_file[addr];
            else
                reg_file[addr] = data;
            return apb_resp::SUCCESS;
        } else
            return apb_resp::ERROR;
    }
};

SC_MODULE(tb) {

    sc_signal<bool>         clk{"clk"};
    sc_signal<bool>         rstn{"rstn"};
    sc_signal<sc_uint<32>>  paddr{"paddr"};
    sc_signal<bool>         psel{"psel"};
    sc_signal<bool>         penable{"penable"};
    sc_signal<bool>         pwrite{"pwrite"};
    sc_signal<sc_uint<32>>  pwdata{"pwdata"};
    sc_signal<bool>         pready{"pready"};
    sc_signal<sc_uint<32>>  prdata{"prdata"};
    sc_signal<bool>         pslverr{"pslverr"};

    dut<sc_uint<32> , sc_uint<32> , 16> dut_inst{"dut_inst"};

    SC_CTOR(tb) {
        dut_inst.clk(clk);
        dut_inst.rstn(rstn);
        dut_inst.apb_tar.paddr(paddr);
        dut_inst.apb_tar.psel(psel);
        dut_inst.apb_tar.penable(penable);
        dut_inst.apb_tar.pwrite(pwrite);
        dut_inst.apb_tar.pwdata(pwdata);
        dut_inst.apb_tar.pready(pready);
        dut_inst.apb_tar.prdata(prdata);
        dut_inst.apb_tar.pslverr(pslverr);
    };

};


int sc_main(int argc, char **argv) {
    tb tb_inst{"tb_inst"};
    sc_start();
    return 0;
}
