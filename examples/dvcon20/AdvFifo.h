/**
 * Shared memory design.
 * Advanced FIFO collection.
 * 
 * Assertions added.
 *
 * Author: Mikhail Moiseev
 * Copyright (c) 2015 Intel Corporation. All rights reserved.
 */

#ifndef ADV_FIFO_H
#define ADV_FIFO_H

#include "sct_static_log.h"
#include "sct_assert.h"
#include "systemc.h"

/**
 * Advanced FIFO may work in asynchronous or synchronous mode.
 * This FIFO may be used as MCP with define one of isPopEnable() and
 * isPushEnable() methods only.
 *
 * @push and @pop may be asserted whenever.
 * FIFO does push operation when both @push/@ready_to_push asserted.
 * FIFO does pop operation when all three @pop/@out_valid/@pop_enable asserted.
 *
 * @push_enable used for MCP request FIFO and return 1 on matched clocks,
 * it restricts changing @read_to_push and @afull output, but not push operation.
 * @pop_enable used for MCP response FIFO and return 1 on matched clocks.
 *
 * Outputs @out_valid, @ready and @afull may be asserted asynchronously,
 * de-assertion is done synchronously only.
 *
 * FIFO allows to pop an element in the same clock it is pushed into empty FIFO,
 * to do that @out_valid and @data_valid are asserted asynchronously.
 * FIFO provides asynchronous assertion @ready_to_push for full FIFO,
 * if pop operation will be done next clock posedge.
 * FIFO provides asynchronous assertion @almost_full when FIFO is one element
 * lower than length and there is push and no pop. This feature is not MCP ready.
 * De-assertion @out_valid, @ready_to_push, @almost_full is done synchronously!!!
 *
 * Async @out_valid combinationally depends on @push.
 * Async @ready_to_push combinationally depends on @pop.
 * Async @afull combinationally depends on @pop and @push.
 *
 * Asynchronous @out_valid and @ready_to_push works for MCP request/response
 * FIFO as well.
 */

template <typename T,
          bool     ASYNC_VALID,  // Set @out_valid asynchronously
          bool     ASYNC_READY,  // Set @ready_to_push asynchronously
          bool     ASYNC_AFULL,  // Set @almost_full asynchronously
          unsigned FIFO_LENGTH,
          unsigned AFULL_ELEMENT_NUM,  // Number of free element slots in FIFO
                                       // when @is_almost_full is set high
          bool INIT_BUFFER             // Initialize @fifo_buffer in reset
          >
class adv_fifo_base : public sc_module
{
   public:
    sc_in_clk   clk;
    sc_in<bool> nrst;
    // Push operation is performed without @enable checking for burst on core clock,
    // @push may be asserted when @ready_to_push is high only
    sc_in<bool> push;
    sc_in<T>    data_in;
    // FIFO is ready to @push assert signal
    sc_out<bool> ready_to_push;
    // @pop may be asserted whenever, pop operation is done when @pop && @out_valid
    sc_in<bool> pop;
    sc_out<T>   data_out;
    // @data_out is valid signal
    sc_out<bool> out_valid;
    sc_out<bool> almost_full;

    SC_HAS_PROCESS(adv_fifo_base);

   protected:
    // Do not create this class instance, use child classes
    explicit adv_fifo_base(const sc_module_name& name)
        : sc_module(name)
        , clk("clk")
        , nrst("nrst")
        , push("push")
        , data_in("data_in")
        , ready_to_push("ready_to_push")
        , pop("pop")
        , data_out("data_out")
        , out_valid("out_valid")
        , almost_full("almost_full")
        , element_num("element_num")
        , pop_data("pop_data")
        , push_sig("push_sig")
        , pop_enable("pop_enable")
        , push_enable("push_enable")
        , pop_enable_sig("pop_enable_sig")
        , push_enable_sig("push_enable_sig")
        , mcp_data_reg("mcp_data_reg")
        , mcp_valid_reg("mcp_valid_reg")
        , out_valid_reg("out_valid_reg")
        , ready_push_reg("ready_push_reg")
        , almost_full_reg("almost_full_reg")
    {
        // For async @afull there must be and async valid or FIFO size more than
        // 1, to guarantee that if asserted @pop, FIFO is not almost full, i.e.
        // if @pop is asserted pop operation happens in all cases.
        assert(!ASYNC_AFULL || (ASYNC_VALID || FIFO_LENGTH > 1));

        if (FIFO_LENGTH <= AFULL_ELEMENT_NUM) {
            cout << "FIFO name " << name << ", length = " << FIFO_LENGTH
                 << ", alfull = " << AFULL_ELEMENT_NUM << endl;
            SC_REPORT_ERROR("", "Incorrect FIFO parameters");
        }

        SC_METHOD(asyncProc);
        sensitive << element_num << data_in << push << pop << out_valid_reg
                  << mcp_data_reg << mcp_valid_reg << pop_enable
                  << pop_enable_sig << pop_data << ready_push_reg
                  << push_enable_sig << almost_full_reg;

        SC_CTHREAD(syncProc, clk.pos());
        async_reset_signal_is(nrst, false);
    }

   protected:
    // Number of bits in variables store length or index of FIFO
    static const int FIFO_INDX_WIDE = StaticLog::bits_one<FIFO_LENGTH>::value;

    // FIFO buffer (one additional element that is not used to prevent
    // read and write of the same cell in one clock tick)
    T fifo_buffer_flat[FIFO_LENGTH];
    // Index of element that will be poped
    sc_uint<FIFO_INDX_WIDE> popIndx;
    // Index where pushed element will be stored
    sc_uint<FIFO_INDX_WIDE> pushIndx;
    // Number of elements, +1 because it in [0..FIFO_LENGTH] range
    sc_uint<FIFO_INDX_WIDE + 1> elementNum;
    // Signals to pop and push data to buffer
    sc_signal<T> pop_data;
    // Number of elements
    sc_signal<sc_uint<FIFO_INDX_WIDE + 1> > element_num;
    sc_signal<sc_uint<FIFO_INDX_WIDE + 1> > element_num_d;
    // Push data to buffer
    sc_signal<bool> push_sig;
    // Enable pop operation and asynchronous @out_valid/@data_out and
    // @ready_to_push update
    sc_signal<bool> pop_enable;
    sc_signal<bool> push_enable;
    // Delayed one clock tick @pop_enable/@push_enable
    sc_signal<bool> pop_enable_sig;
    sc_signal<bool> push_enable_sig;
    // Registers to store outputs in MCP mode
    sc_signal<T>    mcp_data_reg;
    sc_signal<bool> mcp_valid_reg;
    // Register to store @out_valid in normal and MCP modes
    sc_signal<bool> out_valid_reg;
    // Register to store @ready_to_push in normal and MCP modes
    sc_signal<bool> ready_push_reg;
    // Register to store @almost_full
    sc_signal<bool> almost_full_reg;

    // Push leads to non-empty FIFO
    SCT_ASSERT(push && !ready_to_push && (!pop || !out_valid), SCT_TIME(1), 
               ready_to_push, clk.pos());
    // Full FIFO has AFULL flag and does not get new elements
    SCT_ASSERT(element_num.read() == FIFO_LENGTH, SCT_TIME(0), 
               almost_full && !ready_to_push, clk.pos());
    // Element number not changed if pop and push both happen
    SCT_ASSERT(push && ready_to_push && pop && out_valid, SCT_TIME(1), 
               element_num == element_num_d, clk.pos());
    // Element number changed if pop or push both happen
    SCT_ASSERT((!push || !ready_to_push) && pop && out_valid, SCT_TIME(1), 
               element_num.read() == element_num_d.read()-1, clk.pos());
    
    void asyncProc()
    {
        if (ASYNC_VALID) {
            if (!out_valid_reg) {
                if (pop_enable_sig) {
                    // In MCP response mode @pop is stable whole slow clock period,
                    // so current value will be taken next posedge and may be not pushed
                    data_out  = data_in.read();
                    out_valid = push;
                    push_sig  = push && !pop;
                    //                    if (!strcmp(this->name(),
                    //                    "tb.top.org_base_port.mcp_rresp_fifo"))
                    //                    cout << sc_time_stamp() << "," <<name()
                    //                    << ", get input, valid = "
                    //                         << push << ", data "<< data_in.read() << endl;
                } else {
                    // This code is active for MCP response only,
                    // use registers to avoid glitch on MCP outputs
                    out_valid = mcp_valid_reg;
                    data_out  = mcp_data_reg.read();
                    // Any new input data must be pushed, despite is there or
                    // not previous data is stored in @data_out_reg
                    push_sig = push;
                    //                    if (!strcmp(this->name(),
                    //                    "tb.top.org_base_port.mcp_rresp_fifo"))
                    //                    cout << sc_time_stamp() << ","
                    //                    <<name() << ", get out_reg, valid = "
                    //                         << mcp_valid_reg << ", data "<<
                    //                         mcp_data_reg.read() << endl;
                }

            } else {
                out_valid = 1;
                data_out  = pop_data.read();
                push_sig  = push;
                //                if (!strcmp(this->name(),
                //                "tb.top.org_base_port.mcp_rresp_fifo")) cout
                //                << sc_time_stamp() <<","<< name() << ", get
                //                stored "<< pop_data.read()
                //                     << ", num = " << element_num.read() <<
                //                     endl;
            }
        } else {
            // @out_valid is stable if @element_num is not changed
            out_valid = out_valid_reg;
            // @data_out may be changed, but it is read by @out_valid, therefore skipped
            data_out = pop_data.read();
            push_sig = push;
        }

        // @ready_to_push is always 0 during reset
        if (ASYNC_READY) {
            // @ready_to_push may be changed when @push_enable_sig is high only
            if (push_enable_sig) {
                // Check @pop_enable and (element_num.read() != 0) to avoid
                // assert @ready_to_push for MCP response FIFO if pop will
                // not be done next posedge
                // Use @ready_push_reg instead of (elementNum != FIFO_LENGTH)
                ready_to_push = pop && pop_enable && out_valid_reg ||
                                ready_push_reg;
            } else {
                ready_to_push = ready_push_reg;
            }
        } else {
            ready_to_push = ready_push_reg;
        }

        // Asynchronously update @is_almost_full to reduce latency
        // @almost_full is always 0 during reset
        if (ASYNC_AFULL) {
            // Set earlier to reduce latency and cut off FIFO length
            // Not checked to MCP compliance, there is assert in MCP FIFOs
            // If @pop is asserted then FIFO does not become almost full, that is right
            // for ASYNC_VALID or FIFO_LENGTH > 1 FIFO, checked in constructor.
            almost_full =
                almost_full_reg ||
                (push && !pop &&
                 element_num.read() == FIFO_LENGTH - AFULL_ELEMENT_NUM - 1);

        } else {
            almost_full = almost_full_reg;
        }
    }

    void syncProc()
    {
        element_num     = 0;
        element_num_d   = 0;
        out_valid_reg   = 0;
        almost_full_reg = 0;
        // Set zero to avoid after reset change at non-matched clocks
        ready_push_reg = 0;

        // No push and pop is asserted in reset,
        // use 1 here avoids of using MCP registers for non-MCP FIFO
        pop_enable_sig  = 1;
        push_enable_sig = 1;

        mcp_valid_reg = 0;
        mcp_data_reg  = 0;

        elementNum = 0;
        popIndx    = 0;
        pushIndx   = 0;
        pop_data   = 0;

        // Initialize zero cell to provide zero data before first push
        fifo_buffer_flat[0] = T(0);
        // Initialize other cells
        if (INIT_BUFFER) {
        LOOP_UNROLL1:
            for (int i = 1; i < FIFO_LENGTH; i++) {
                fifo_buffer_flat[i] = T(0);
            }
        }
        wait();

        while (true) {
            // Set these signals one clock tick after @clock_match
            pop_enable_sig  = pop_enable;
            push_enable_sig = push_enable;

            mcp_valid_reg  = out_valid.read();
            mcp_data_reg   = data_out.read();
            ready_push_reg = ready_to_push.read();

            // Pop from non-empty FIFO,
            // Use @element_num to avoid pop from MCP FIFO which element(s) is
            // just pushed during not enable priod and @out_valid is not asserted yet
            // That also protects from pop when element is in @mcp_data_reg

            // This pop required if async @ready_to_push to release cell before
            // push, for ASYNC_READY that provides continuous push/pop when FIFO
            // is full for SYNC_READY that is just normal pop
            if (pop && pop_enable && out_valid_reg) {
                if (FIFO_LENGTH > 1) {
                    if (popIndx == FIFO_LENGTH - 1) {
                        popIndx = 0;
                    } else {
                        popIndx += 1;
                    }
                    elementNum -= 1;
                } else {
                    elementNum = 0;
                }
            }

            // Push
            if (push_sig && ready_to_push) {
                assert(elementNum < FIFO_LENGTH);

                fifo_buffer_flat[pushIndx] = data_in.read();
                if (FIFO_LENGTH > 1) {
                    if (pushIndx == FIFO_LENGTH - 1) {
                        pushIndx = 0;
                    } else {
                        pushIndx += 1;
                    }
                    elementNum += 1;
                } else {
                    elementNum = 1;
                }
            }

            // Set output value after push is performed,
            // to cope with push into empty FIFO case (used in MCP also)
            pop_data.write(fifo_buffer_flat[popIndx]);

            // FIFO has @AFULL_ELEMENT_NUM empty slots
            if (push_enable) {
                almost_full_reg =
                    (elementNum > FIFO_LENGTH - AFULL_ELEMENT_NUM - 1);
                ready_push_reg = (elementNum != FIFO_LENGTH);
            }
            if (pop_enable) {
                if (ASYNC_AFULL) element_num.write(elementNum);
                out_valid_reg = (elementNum != 0);
            }
            element_num_d = element_num;

            //        if (!strcmp(name(), "tb.top.org_ap2.local_wresp_fifo")) {
            //            cout << sc_time_stamp() << " FIFO num = " <<
            //            elementNum<< endl;
            //        }
            //        if (elementNum > FIFO_LENGTH-(AFULL_ELEMENT_NUM+1)) {
            //            cout << sc_time_stamp() << " FIFO "<< name() <<"
            //            almost full " << endl;
            //        }
            wait();
        }
    }
    
};

//=========================== Non-MCP FIFO ================================
/**
 * Non-MCP FIFO
 */
template <typename T,
          bool     ASYNC_VALID,  // Set @out_valid asynchronously
          bool     ASYNC_READY,  // Set @ready_to_push asynchronously
          bool     ASYNC_AFULL,  // Set @almost_full asynchronously
          unsigned FIFO_LENGTH,
          unsigned AFULL_ELEMENT_NUM,  // Number of free element slots in FIFO
                                       // when @is_almost_full is set up
          bool INIT_BUFFER = 0         // Initialize @fifo_buffer in reset
          >
class adv_fifo
    : public adv_fifo_base<T, ASYNC_VALID, ASYNC_READY, ASYNC_AFULL,
                           FIFO_LENGTH, AFULL_ELEMENT_NUM, INIT_BUFFER>
{
   public:
    SC_HAS_PROCESS(adv_fifo);

    explicit adv_fifo(const sc_module_name& name)
        : adv_fifo_base<T, ASYNC_VALID, ASYNC_READY, ASYNC_AFULL, FIFO_LENGTH,
                        AFULL_ELEMENT_NUM, INIT_BUFFER>(name)
    {
        assert(FIFO_LENGTH > 0);

        SC_METHOD(setEnableProc);
    }

   protected:
    void setEnableProc()
    {
        this->pop_enable  = 1;
        this->push_enable = 1;
    }
};

/**
 * Non-MCP FIFO with function interface
 */
template <typename T,
          bool     ASYNC_VALID,  // Set @out_valid asynchronously
          bool     ASYNC_READY,  // Set @ready_to_push asynchronously
          bool     ASYNC_AFULL,  // Set @almost_full asynchronously
          unsigned FIFO_LENGTH,
          unsigned AFULL_ELEMENT_NUM,  // Number of free element slots in FIFO
                                       // when @is_almost_full is set up
          bool INIT_BUFFER = 0         // Initialize @fifo_buffer in reset
          >
class adv_fifo_mif : public sc_module, public sc_interface
{
   protected:
    using FifoType = adv_fifo<T, ASYNC_VALID, ASYNC_READY, ASYNC_AFULL,
                              FIFO_LENGTH, AFULL_ELEMENT_NUM, INIT_BUFFER>;
    FifoType fifo;

   public:
    sc_in_clk   clk;
    sc_in<bool> nrst;

    sc_signal<bool> push_sig;
    sc_signal<T>    data_in_sig;
    sc_signal<bool> ready_sig;
    sc_signal<bool> pop_sig;
    sc_signal<T>    data_out_sig;
    sc_signal<bool> valid_sig;
    sc_signal<bool> full_sig;

    SC_HAS_PROCESS(adv_fifo_mif);

    explicit adv_fifo_mif(const sc_module_name& name)
        : sc_module(name)
        , fifo(std::string(std::string("fifo") + (const char*)name).c_str())
        , clk("clk")
        , nrst("nrst")
        , push_sig("push_sig")
        , data_in_sig("data_in_sig")
        , ready_sig("ready_sig")
        , pop_sig("pop_sig")
        , data_out_sig("data_out_sig")
        , valid_sig("valid_sig")
        , full_sig("full_sig")
    {
        fifo.clk(clk);
        fifo.nrst(nrst);
        fifo.push(push_sig);
        fifo.data_in(data_in_sig);
        fifo.ready_to_push(ready_sig);
        fifo.pop(pop_sig);
        fifo.data_out(data_out_sig);
        fifo.out_valid(valid_sig);
        fifo.almost_full(full_sig);
    }

    /// Is ready to push
    bool ready() { return ready_sig; }

    /// Push or clear push, push is ignored is FIFO is not ready to push
    /// \return ready to push flag
    bool push(const T& data, bool push = true)
    {
        data_in_sig = data;
        push_sig    = push;
        return ready_sig;
    }

    /// Is data valid, pop return can be used only if data is valid
    bool valid() { return valid_sig; }

    /// Pop or get data
    T pop(bool pop = true)
    {
        pop_sig = pop;
        return (T)data_out_sig.read();
    }

    /// Is almost full, there is AFULL_ELEMENT_NUM elements or more used
    bool full() { return full_sig; }

    /// Add FIFO signals to sensitivity list
    void addTo(sc_sensitive& s)
    {
        s << ready_sig << valid_sig << data_out_sig << full_sig;
    }

    template <typename CLK_t, typename RSTN_t>
    void clk_nrst(CLK_t& clk_in, RSTN_t& nrst_in)
    {
        clk(clk_in);
        nrst(nrst_in);
    }
};

//======================= Zero size non-MCP FIFO =============================
/**
 * Advanced FIFO in asynchronous mode with zero size
 * Do not use as base class for MCP FIFO
 */
template <typename T>
class adv_fifo<T, 1, 1, 1, 0, 0, 0> : public sc_module
{
   public:
    sc_in_clk   clk;
    sc_in<bool> nrst;
    // Level active push signal
    sc_in<bool>  push;
    sc_in<T>     data_in;
    sc_out<bool> ready_to_push;  // FIFO is ready to push,
                                 // i.e. ready to input signals changed
    // Level active pop signal
    sc_in<bool>  pop;
    sc_out<T>    data_out;
    sc_out<bool> out_valid;
    sc_out<bool> almost_full;

    SC_HAS_PROCESS(adv_fifo);

    explicit adv_fifo(const sc_module_name& name)
        : sc_module(name)
        , clk("clk")
        , nrst("nrst")
        , push("push")
        , data_in("data_in")
        , ready_to_push("ready_to_push")
        , pop("pop")
        , data_out("data_out")
        , out_valid("out_valid")
        , almost_full("almost_full")
    {
        SC_METHOD(asyncProc);
        sensitive << push << pop << data_in;
    }

   protected:
    void asyncProc()
    {
        almost_full   = 1;
        out_valid     = push;
        data_out      = data_in.read();
        ready_to_push = pop;
    }
};

//======================= MCP FIFOs =============================
/**
 * MCP REQUEST FIFO
 * This module NOT used for now.
 * @enable high allows to change @is_almost_full and @ready_to_push outputs,
 * push may happen if @enable is low!!!
 */
template <typename T,
          bool     ASYNC_VALID,  // Set @out_valid asynchronously
          bool     ASYNC_READY,  // Set @ready_to_push asynchronously
          bool     ASYNC_AFULL,  // Set @almost_full asynchronously
          unsigned FIFO_LENGTH,
          unsigned AFULL_ELEMENT_NUM,  // Number of free element slots in FIFO
                                       // when @is_almost_full is set up
          bool INIT_BUFFER = 0         // Initialize @fifo_buffer in reset
          >
class mcp_request_fifo
    : public adv_fifo_base<T, ASYNC_VALID, ASYNC_READY, ASYNC_AFULL,
                           FIFO_LENGTH, AFULL_ELEMENT_NUM, INIT_BUFFER>
{
   public:
    // Enable to change @is_almost_full output
    sc_in<bool> enable;

    SC_HAS_PROCESS(mcp_request_fifo);

    explicit mcp_request_fifo(const sc_module_name& name)
        : adv_fifo_base<T, ASYNC_VALID, ASYNC_READY, ASYNC_AFULL, FIFO_LENGTH,
                        AFULL_ELEMENT_NUM, INIT_BUFFER>(name)
        , enable("enable")
    {
        assert(FIFO_LENGTH > 0);
        assert(!ASYNC_AFULL);  // not supported yet

        SC_METHOD(setEnableProc);
        this->sensitive << enable;
    }

   protected:
    void setEnableProc()
    {
        this->pop_enable  = 1;
        this->push_enable = enable;
    }
};

/**
 * MCP RESPONSE FIFO
 * This module used for response FIFOs in MCP adapter.
 *
 * @enable high allows to change @out_valid outputs,
 * @data_out may be changed without @enable, but it is read by @out_valid,
 * therefore skipped.
 */
template <typename T,
          bool     ASYNC_VALID,  // Set @out_valid asynchronously
          bool     ASYNC_READY,  // Set @ready_to_push asynchronously
          bool     ASYNC_AFULL,  // Set @almost_full asynchronously
          unsigned FIFO_LENGTH,
          unsigned AFULL_ELEMENT_NUM,  // Number of free element slots in FIFO
                                       // when @is_almost_full is set up
          bool INIT_BUFFER = 0         // Initialize @fifo_buffer in reset
          >
class mcp_response_fifo
    : public adv_fifo_base<T, ASYNC_VALID, ASYNC_READY, ASYNC_AFULL,
                           FIFO_LENGTH, AFULL_ELEMENT_NUM, INIT_BUFFER>
{
   public:
    // Enable to change @out_valid output
    sc_in<bool> enable;

    SC_HAS_PROCESS(mcp_response_fifo);

    explicit mcp_response_fifo(const sc_module_name& name)
        : adv_fifo_base<T, ASYNC_VALID, ASYNC_READY, ASYNC_AFULL, FIFO_LENGTH,
                        AFULL_ELEMENT_NUM, INIT_BUFFER>(name)
        , enable("enable")
    {
        // Asynchronous response FIFO does not meet MCP constraints
        assert(FIFO_LENGTH > 0);
        assert(!ASYNC_AFULL);  // not supported yet

        SC_METHOD(setEnableProc);
        this->sensitive << enable;
    }

   protected:
    void setEnableProc()
    {
        this->pop_enable  = enable;
        this->push_enable = 1;
    }
};

#endif /* ADV_FIFO_H */
