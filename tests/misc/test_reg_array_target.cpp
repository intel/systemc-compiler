/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Adopted example with array of registers for SVC synthesis. 
// This is target for SVC tool.
//

#include <systemc.h>

enum sc_register_access_mode {
	SC_REG_RW_ACCESS = 0,
	SC_REG_RO_ACCESS,
	SC_REG_WO_ACCESS
};
enum sc_register_access_type {
	SC_REG_UNKNOWN_ACCESS_TYPE,
	SC_REG_READ,
	SC_REG_WRITE
};

template< typename T >
struct sc_register
{
    sc_register() {}
    
    //void reset(const T& reset_val, sc_register_access_mode mode) 
    void reset(T reset_val, sc_register_access_mode mode) 
    {
        m_value = reset_val;
        m_acess_mode = mode;
    }
    
    T read() const {
        return m_value;
    }
    
    void write(const T& new_val)
    {
        m_value = new_val;
    }
    
    sc_register_access_mode get_access_mode() const {
        return m_acess_mode; 
    }
    
private:
    T m_value;    
    sc_register_access_mode m_acess_mode;
};


template <unsigned N>
struct registers_t 
{
    registers_t() : size(N << 2)
    {}
    
    void reset() {
        regs[0].reset(0x8000000, SC_REG_RW_ACCESS);
        regs[1].reset(0x0000000, SC_REG_RW_ACCESS);
        // ...
    }
    
    bool read(unsigned offset, unsigned& res)
    {
        res = 0;

        if (offset >= size) {
            return false;
            
        } else {
            unsigned reg_no = offset >> 2;
            res = regs[reg_no].read();
            return true;
        }
    }
    
    bool write(unsigned offset, unsigned val)
    {
        if (offset >= size) {
            return false;
            
        } else {
            unsigned reg_no = offset >> 2;
            regs[reg_no].write(val);
            return true;
        }
    }
    
    size_t get_size() const
    {
        return size;
    }
    
private:
    const size_t size;
    sc_register<unsigned>  regs[N];
};

SC_MODULE(Top)
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rst{"rst"};
    sc_in<bool>         renable{"renable"};
    sc_in<bool>         wenable{"wenable"};
    sc_in<unsigned>     addr{"addr"};
    sc_in<unsigned>     data_in{"data_in"};
    sc_out<unsigned>    data_out{"data_out"};
    
    registers_t<2>      registers;
    
    SC_CTOR(Top) {
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(rst, false);
    }
    
    // Access registers
    void proc() 
    {
        data_out = 0;
        registers.reset();
        wait();
            
        while (true) 
        {
            if (renable) {
                unsigned res;
                registers.read(addr, res);
                data_out = res;
                
            } else
            if (wenable) {
                registers.write(addr, data_in);
            }
            
            wait();
        }
    }
};

int sc_main(int argc, char** argv) 
{
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> rst;
    sc_signal<bool> renable;
    sc_signal<bool> wenable;
    sc_signal<unsigned> addr;
    sc_signal<unsigned> data_in;
    sc_signal<unsigned> data_out;
    
    Top top("top");
    top.clk(clk);
    top.rst(rst);
    top.renable(renable);
    top.wenable(wenable);
    top.addr(addr);
    top.data_in(data_in);
    top.data_out(data_out);
    
    sc_start();
    
    return 0;
}
