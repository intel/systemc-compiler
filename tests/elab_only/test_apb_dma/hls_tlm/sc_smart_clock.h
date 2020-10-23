/*****************************************************************************

  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2014 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.accellera.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.

 *****************************************************************************/

/**
 * Clock with event generation disabled by input CG enable signals.
 */

#ifndef SC_SMART_CLOCK_H
#define SC_SMART_CLOCK_H

#include <stdio.h>
#include "sysc/kernel/sc_module.h"
#include "sysc/communication/sc_signal.h"
#include "sysc/tracing/sc_trace.h"

namespace sc_core {

class sc_smart_clock
  : public sc_signal<bool,SC_ONE_WRITER>
{
  typedef sc_signal<bool,SC_ONE_WRITER> base_type;
public:

    friend class sc_smart_clock_posedge_callback;
    friend class sc_smart_clock_negedge_callback;
    friend class sc_smart_clock_control_callback;

    // constructors

    sc_smart_clock();

    explicit sc_smart_clock( const char* name_ );

    sc_smart_clock( const char* name_,
	      const sc_time& period_,
	      double         duty_cycle_ = 0.5,
	      const sc_time& start_time_ = SC_ZERO_TIME,
	      bool           posedge_first_ = true );

    sc_smart_clock( const char* name_,
	      double         period_v_,
	      sc_time_unit   period_tu_,
	      double         duty_cycle_ = 0.5 );

    sc_smart_clock( const char* name_,
	      double         period_v_,
	      sc_time_unit   period_tu_,
	      double         duty_cycle_,
	      double         start_time_v_,
	      sc_time_unit   start_time_tu_,
	      bool           posedge_first_ = true );

    // for backward compatibility with 1.0
    sc_smart_clock( const char* name_,
	      double         period_,            // in default time units
	      double         duty_cycle_ = 0.5,
	      double         start_time_ = 0.0,  // in default time units
	      bool           posedge_first_ = true );

    // destructor (does nothing)
    virtual ~sc_smart_clock();

    virtual void register_port( sc_port_base&, const char* if_type );
    virtual void write( const bool& );

    // get the period
    const sc_time& period() const
	{ return m_period; }

    // get the duty cycle
    double duty_cycle() const
	{ return m_duty_cycle; }


    // get the current time / clock characteristics

    bool posedge_first() const
        { return m_posedge_first; }

    sc_time start_time() const
        { return m_start_time; }

    static const sc_time& time_stamp();

    virtual const char* kind() const
        { return "sc_clock"; }

    // Register enable CG signal/port
    void register_cg_enable(sc_signal_inout_if<bool>& enable);
    
protected:
    // Enable/disable clock 
    void control_action();

    void before_end_of_elaboration();

    // processes
    void posedge_action();
    void negedge_action();

    // error reporting
    void report_error( const char* id, const char* add_msg = 0 ) const;

    void init( const sc_time&, double, const sc_time&, bool );

    bool is_clock() const { return true; }

protected:

    sc_time  m_period;		// the period of this clock
    double   m_duty_cycle;	// the duty cycle (fraction of period)
    sc_time  m_start_time;	// the start time of the first edge
    bool     m_posedge_first;   // true if first edge is positive
    sc_time  m_posedge_time;	// time till next positive edge
    sc_time  m_negedge_time;	// time till next negative edge

    sc_event m_next_posedge_event;
    sc_event m_next_negedge_event;
    sc_event m_check_enable_start;

    std::vector<sc_signal_inout_if<bool>*>  cg_enables;  // CG enable events
    bool m_clock_enable  = true;      // Clock enable flag
    char m_start_clock   = true;      // Clock start flag
    
private:
    // Disabled
    sc_smart_clock( const sc_smart_clock& );
    sc_smart_clock& operator = ( const sc_smart_clock& );
};

// ----------------------------------------------------------------------------

inline void sc_smart_clock::posedge_action()
{
    //std::cout << sc_time_stamp() << "," << sc_delta_count() << ", posedge_action " << m_clock_enable << std::endl;
    // Initiate @m_clock_enable computation at start 
    if (m_start_clock && m_posedge_first) {
        m_start_clock = false;
        // Use DC notification to get signal value assigned in reset at 0 DC
        m_check_enable_start.notify(SC_ZERO_TIME);
    }
    if (m_clock_enable) {
        //m_next_negedge_event.notify_internal( m_negedge_time );
        m_next_negedge_event.notify( m_negedge_time );
        m_new_val = true;
        request_update();
    }    
}

inline void sc_smart_clock::negedge_action()
{
    //std::cout << sc_time_stamp() << "," << sc_delta_count() << ", negedge_action" << std::endl;
    //m_next_posedge_event.notify_internal( m_posedge_time );
    m_next_posedge_event.notify( m_posedge_time );
    m_new_val = false;
    request_update();
}

// ----------------------------------------------------------------------------

class sc_smart_clock_posedge_callback {
public:
    sc_smart_clock_posedge_callback(sc_smart_clock* target_p) : m_target_p(target_p) {}
    inline void operator () () { m_target_p->posedge_action(); }
  protected:
    sc_smart_clock* m_target_p;
};

class sc_smart_clock_negedge_callback {
  public:
    sc_smart_clock_negedge_callback(sc_smart_clock* target_p) : m_target_p(target_p) {}
    inline void operator () () { m_target_p->negedge_action(); }
  protected:
    sc_smart_clock* m_target_p;
};

class sc_smart_clock_control_callback {
  public:
    sc_smart_clock_control_callback(sc_smart_clock* target_p) : m_target_p(target_p) {}
    inline void operator () () { m_target_p->control_action(); }
  protected:
    sc_smart_clock* m_target_p;
};

} // namespace sc_core


#endif

