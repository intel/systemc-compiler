/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Single Source library. Controlled clock generator.
 * 
 * Clock events can be disabled and enable by function call or by clock gate 
 * signals/ports bound. 
 * Approximate time implementation provides notification aligned with other 
 * channels events that allows to mix approximate time and cycle accurate modules.
 *    
 * Author: Mikhail Moiseev
 */

#ifndef SCT_CLOCK_H
#define SCT_CLOCK_H

#include "sct_ipc_if.h"
#include "sysc/kernel/sc_spawn.h"
#include <systemc.h>

namespace sct {
    
/// Cycle accurate implementation
template<>   
class sct_clock<0> : 
    public sc_clock
{
  public:
    class sct_posedge_callback {
      protected:
        sct_clock* m_clock;

      public:
        inline sct_posedge_callback(sct_clock* m_clock_) : m_clock(m_clock_) {}
        inline void operator () () {m_clock->posedge_action();}
    };

    class sct_negedge_callback {
      protected:
        sct_clock* m_clock;

      public:
        inline sct_negedge_callback(sct_clock* m_clock_) : m_clock(m_clock_) {}
        inline void operator () () {m_clock->negedge_action();}
    };

    class sct_control_callback {
      protected:
        sct_clock* m_clock;

      public:
        inline sct_control_callback(sct_clock* m_clock_) : m_clock(m_clock_) {}
        inline void operator () () {m_clock->control_action();}
    };

    
    inline sct_clock() : sc_clock() 
    {}
    
    inline explicit sct_clock(const char* name_) : sc_clock(name_)
    {}
    
    /// Clock enabled by default, cannot be disabled in RTL mode
    inline sct_clock(const char*    name_,
                     const sc_time& period_,
                     bool           enabled = true,
                     double         duty_cycle_ = 0.5,
                     const sc_time& start_time_ = SC_ZERO_TIME,
                     bool           posedge_first_ = true)
      : sc_clock(name_, period_, duty_cycle_, start_time_, posedge_first_)
    {}
            
    inline sct_clock(const char*   name_,
                     double        period_v_,
                     sc_time_unit  period_tu_,
                     bool          enabled = true,
                     double        duty_cycle_ = 0.5)
      : sc_clock(name_, period_v_, period_tu_, duty_cycle_)
    {}
    
    inline sct_clock(const char*   name_,
                     double        period_v_,
                     sc_time_unit  period_tu_,
                     double        duty_cycle_,
                     double        start_time_v_,
                     sc_time_unit  start_time_tu_,
                     bool          posedge_first_ = true)
      : sc_clock(name_, period_v_, period_tu_, duty_cycle_, start_time_v_,
                         start_time_tu_, posedge_first_)
    {}
    
    inline sct_clock(const char*   name_,
                     double        period_,            // in default time units
                     double        duty_cycle_ = 0.5,
                     double        start_time_ = 0.0,  // in default time units
                     bool          posedge_first_ = true)
      : sc_clock(name_, period_, duty_cycle_, start_time_, posedge_first_)
    {}
    
    /// Enable clock activity, clock is enabled after construction 
    void enable() {
        //std::cout << sc_time_stamp() << " enable" << std::endl;
        assert(cg_enables.empty() && "No manual control allowed as CG inputs bound");

        if (!m_clock_enable) {
            if (m_clock_disabled) {
                //std::cout << sc_time_stamp() << " +" << std::endl;
                sc_clock::posedge_action();
            }
            m_clock_disabled = 0;
        }
        m_clock_enable = 1;
    }
    
    /// Disable clock activity, can be called at elaboration phase to disable
    /// clock at simulation phase start
    void disable() {
        //std::cout << sc_time_stamp() << " disable" << std::endl;
        assert(cg_enables.empty() && "No manual control allowed as CG inputs bound");

        m_clock_enable = 0;
    }
    
    /// Register clock gate signals/ports to control clock activity.
    /// If any of the signals/ports is high clock is enabled
    void register_cg_enable(sc_signal_inout_if<bool>& enable) {
        cg_enables.push_back(&enable);        
    }
    
  protected:    
    /// Clock is currently enabled 
    bool m_clock_enable = 1;
    /// Clock is disabled and posedge event has been skipped
    bool m_clock_disabled = 0;
    /// Clock gate signals/ports
    std::vector<sc_signal_inout_if<bool>*>  cg_enables;
    
    void posedge_action() {
        if (m_clock_enable) {
            //std::cout << sc_time_stamp() << " +" << std::endl;
            sc_clock::posedge_action();
        } else {
            m_clock_disabled = 1;
        }
    }
    
    void negedge_action() {
        //std::cout << sc_time_stamp() << " -" << std::endl;
        sc_clock::negedge_action();
    }
    
    void control_action() {
        if (!cg_enables.empty()) {
            bool enable = 0;
            for (auto& s : cg_enables) {
                enable = enable || s->read();
            }
            //std::cout << sc_time_stamp() << " control_action " << enable << std::endl;

            if (enable && !m_clock_enable) {
                if (m_clock_disabled) {
                    //std::cout << sc_time_stamp() << " +" << std::endl;
                    sc_clock::posedge_action();
                }
                m_clock_disabled = 0;
            }
            m_clock_enable = enable;
        }
    }
    
    void before_end_of_elaboration() override {
        std::string name_str;
        sc_spawn_options p_options; 
        p_options.spawn_method();
        p_options.dont_initialize();
        p_options.set_sensitivity(&m_next_posedge_event);

        name_str = basename() + std::string("_posedge_action");
        sc_spawn(sct_posedge_callback(this),
                 sc_gen_unique_name(name_str.c_str()), &p_options);

        sc_spawn_options n_options; 
        n_options.spawn_method();
        n_options.dont_initialize();
        n_options.set_sensitivity(&m_next_negedge_event);

        name_str = basename() + std::string("_negedge_action");
        sc_spawn(sct_negedge_callback(this),
                sc_gen_unique_name(name_str.c_str()), &n_options);

        if (!cg_enables.empty()) {
            sc_spawn_options enable_options;
            enable_options.spawn_method();
            for (auto& s : cg_enables) {
                enable_options.set_sensitivity(&(s->value_changed_event()));
            }

            name_str = basename() + std::string("_enable_action");
            sc_spawn(sct_control_callback(this),
                     sc_gen_unique_name(name_str.c_str()), &enable_options);
        }        
    }
};

//==============================================================================

/// Approximate time implementation
template<>    
class sct_clock<1> : 
    public sc_prim_channel, 
    public sc_signal_in_if<bool>, 
    public sct_clock_if
{
  public:
    class sct_edge_callback {
      protected:
        sct_clock* m_clock;

      public:
        inline sct_edge_callback(sct_clock* m_clock_) : m_clock(m_clock_) {}
        inline void operator () () {m_clock->edge_action();}
    };

    class sct_control_callback {
      protected:
        sct_clock*  m_clock;

      public:
        inline sct_control_callback(sct_clock* m_clock_) : m_clock(m_clock_) {}
        inline void operator () () {m_clock->control_action();}
    };
    
  public:
    /// Clock enabled by default, could be disabled to provide clock period only
    inline sct_clock(const char* name, 
                     const sc_time& period,
                     bool enabled = true) : 
        sc_prim_channel(name), 
        m_period(period),
        m_event(std::string(std::string(name)+"_event").c_str())
    {
        if (enabled) m_event.notify(SC_ZERO_TIME);
    }
    
    inline sct_clock(const char* name,
                     double period_v, 
                     sc_time_unit  period_tu,
                     bool enabled = true) : 
        sc_prim_channel(name), 
        m_period(sc_time(period_v, period_tu)),
        m_event(std::string(std::string(name)+"_event").c_str())
    {
        if (enabled) m_event.notify(SC_ZERO_TIME);
    }
    
    /// Enable clock activity, clock is enabled after construction 
    void enable() {
        //std::cout << sc_time_stamp() << " enable" << std::endl;
        assert(cg_enables.empty() && "No manual control allowed as CG inputs bound");

        if (!m_clock_enable) {
            //std::cout << sc_time_stamp() << " +" << std::endl;
            m_event.notify(m_period);
        }
        m_clock_enable = 1;
    }
    
    /// Disable clock activity, can be called at elaboration phase to disable
    /// clock at simulation phase start
    void disable() {
        //std::cout << sc_time_stamp() << " disable" << std::endl;
        assert(cg_enables.empty() && "No manual control allowed as CG inputs bound");

        m_clock_enable = 0;
    }
    
    /// Register clock gate signals/ports to control clock activity.
    /// If any of the signals/ports is high, then clock is enabled
    void register_cg_enable(sc_signal_inout_if<bool>& enable) {
        cg_enables.push_back(&enable);
    }
    
    const sc_time& period() const override {
        return m_period;
    }
    
  protected:    
    /// Clock is currently enabled 
    bool m_clock_enable = 1;
    /// Clock gate signals/ports
    std::vector< sc_signal_inout_if<bool>* >  cg_enables;
    /// Clock period
    const sc_time   m_period;      
    sc_event        m_event;

    inline void edge_action() {
        if (m_clock_enable) {
            //std::cout << sc_time_stamp() << " +" << std::endl;
            m_event.notify(m_period);
        }
    }
    
    inline void control_action() 
    {
        if (!cg_enables.empty()) {
            bool enable = 0;
            for (auto& s : cg_enables) {
                enable = enable || s->read();
            }
            //std::cout << sc_time_stamp() << " control_action " << enable << std::endl;

            if (enable && !m_clock_enable) {
                //std::cout << sc_time_stamp() << " +" << std::endl;
                m_event.notify(m_period);
            }
            m_clock_enable = enable;
        }
    }
    
    void before_end_of_elaboration() override
    {
        std::string name_str;
        sc_spawn_options edge_options;  
        edge_options.spawn_method();
        edge_options.dont_initialize();
        edge_options.set_sensitivity(&m_event);

        name_str = basename() + std::string("_edge_action");
        sc_spawn(sct_edge_callback(this),
                 sc_gen_unique_name(name_str.c_str()), &edge_options);

        if (!cg_enables.empty()) {
            sc_spawn_options enable_options;
            enable_options.spawn_method();
            for (auto& s : cg_enables) {
                enable_options.set_sensitivity(&(s->value_changed_event()));
            }

            name_str = basename() + std::string("_enable_action");
            sc_spawn(sct_control_callback(this),
                     sc_gen_unique_name(name_str.c_str()), &enable_options);
        }
    }
    
 public:  
    inline virtual const sc_event& default_event() const override {
        return m_event;
    }
    
    inline virtual const sc_event& value_changed_event() const override {
        return m_event;
    }

    inline virtual const sc_event& posedge_event() const override {
        return m_event;
    }

    inline virtual const sc_event& negedge_event() const override {
        return m_event;
    }

  private:    
    bool       m_val = 0;

    inline virtual const bool& read() const override {
        return m_val;
    }

    inline virtual const bool& get_data_ref() const override {
        return m_val;
    }

    inline virtual bool event() const override {
        return 0;
    }

    inline virtual bool posedge() const override {
        return 0;
    }

    inline virtual bool negedge() const override {
        return 0;
    }    
};

} // namespace sct

#endif /* SCT_CLOCK_H */

