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

#include <vector>

#include "sc_smart_clock.h"
#include "sysc/communication/sc_communication_ids.h"
#include "sysc/kernel/sc_simcontext.h"
#include "sysc/kernel/sc_process.h"
#include "sysc/kernel/sc_spawn.h"
#include "sysc/utils/sc_utils_ids.h"

namespace sc_core {

// ----------------------------------------------------------------------------
//  CLASS : sc_smart_clock
//
//  The clock channel.
// ----------------------------------------------------------------------------

// constructors

sc_smart_clock::sc_smart_clock() : 
    base_type( sc_gen_unique_name( "clock" ) ),
    m_period(), m_duty_cycle(), m_start_time(), m_posedge_first(),
    m_posedge_time(), m_negedge_time(),
    m_next_posedge_event("next_posedge_event"),
    m_next_negedge_event("next_negedge_event")

{
    init( //sc_time::from_value(simcontext()->m_time_params->default_time_unit),
          sc_time(1, SC_NS),    // Default time unit, see kernel/sc_time.cpp
	  0.5,
	  SC_ZERO_TIME,
	  true );

    //m_next_posedge_event.notify_internal( m_start_time );
    m_next_posedge_event.notify( m_start_time );
}

sc_smart_clock::sc_smart_clock( const char* name_ ) :
    base_type( name_ ),
    m_period(), m_duty_cycle(), m_start_time(), m_posedge_first(),
    m_posedge_time(), m_negedge_time(),
    m_next_posedge_event("next_posedge_event"),
    m_next_negedge_event("next_negedge_event")
{
    init( //sc_time::from_value(simcontext()->m_time_params->default_time_unit),
          sc_time(1, SC_NS),    // Default time unit, see kernel/sc_time.cpp
	  0.5,
	  SC_ZERO_TIME,
	  true );

	//m_next_posedge_event.notify_internal( m_start_time );
        m_next_posedge_event.notify( m_start_time );
}

sc_smart_clock::sc_smart_clock( const char* name_,
		    const sc_time& period_,
		    double         duty_cycle_,
		    const sc_time& start_time_,
		    bool           posedge_first_ ) :
    base_type( name_ ),
    m_period(), m_duty_cycle(), m_start_time(), m_posedge_first(),
    m_posedge_time(), m_negedge_time(),
    m_next_posedge_event("next_posedge_event"),
    m_next_negedge_event("next_negedge_event")
{
    init( period_,
	  duty_cycle_,
	  start_time_,
	  posedge_first_ );

    if( posedge_first_ ) {
	// posedge first
	//m_next_posedge_event.notify_internal( m_start_time );
        m_next_posedge_event.notify( m_start_time );
    } else {
	// negedge first
	//m_next_negedge_event.notify_internal( m_start_time );
        m_next_negedge_event.notify( m_start_time );
    }
}

sc_smart_clock::sc_smart_clock( const char* name_,
		    double         period_v_,
		    sc_time_unit   period_tu_,
		    double         duty_cycle_ ) :
    base_type( name_ ),
    m_period(), m_duty_cycle(), m_start_time(), m_posedge_first(),
    m_posedge_time(), m_negedge_time(),
    m_next_posedge_event("next_posedge_event"),
    m_next_negedge_event("next_negedge_event")
{
    init( sc_time( period_v_, period_tu_, simcontext() ),
	  duty_cycle_,
	  SC_ZERO_TIME,
	  true );

    // posedge first
    //m_next_posedge_event.notify_internal( m_start_time );
    m_next_posedge_event.notify( m_start_time );
}

sc_smart_clock::sc_smart_clock( const char* name_,
		    double         period_v_,
		    sc_time_unit   period_tu_,
		    double         duty_cycle_,
		    double         start_time_v_,
		    sc_time_unit   start_time_tu_,
		    bool           posedge_first_ ) :
    base_type( name_ ),
    m_period(), m_duty_cycle(), m_start_time(), m_posedge_first(),
    m_posedge_time(), m_negedge_time(),
    m_next_posedge_event("next_posedge_event"),
    m_next_negedge_event("next_negedge_event")
{
    init( sc_time( period_v_, period_tu_, simcontext() ),
	  duty_cycle_,
	  sc_time( start_time_v_, start_time_tu_, simcontext() ),
	  posedge_first_ );

    if( posedge_first_ ) {
	// posedge first
	//m_next_posedge_event.notify_internal( m_start_time );
        m_next_posedge_event.notify( m_start_time );
    } else {
	// negedge first
	//m_next_negedge_event.notify_internal( m_start_time );
        m_next_negedge_event.notify( m_start_time );
    }
}

// for backward compatibility with 1.0
sc_smart_clock::sc_smart_clock( const char* name_,
		    double         period_,      // in default time units
		    double         duty_cycle_,
		    double         start_time_,  // in default time units
		    bool           posedge_first_ ) :
    base_type( name_ ),
    m_period(), m_duty_cycle(), m_start_time(), m_posedge_first(),
    m_posedge_time(), m_negedge_time(),
    m_next_posedge_event( "next_posedge_event"),
    m_next_negedge_event( "next_negedge_event")
{
    static bool warn_sc_clock=true;
    if ( warn_sc_clock )
    {
        warn_sc_clock = false;
	SC_REPORT_INFO(SC_ID_IEEE_1666_DEPRECATION_, 
	   "\n    sc_clock(const char*, double, double, double, bool)\n"
	   "    is deprecated use a form that includes sc_time or\n"
	   "    sc_time_unit");
    }

    //sc_time default_time =
    //  sc_time::from_value( simcontext()->m_time_params->default_time_unit );
    
    sc_time default_time(1, SC_NS);    // Default time unit, see kernel/sc_time.cpp;

    init( ( period_ * default_time ),
	  duty_cycle_,
	  ( start_time_ * default_time ),
	  posedge_first_ );

    if( posedge_first_ ) {
	// posedge first
	//m_next_posedge_event.notify_internal( m_start_time );
        m_next_posedge_event.notify( m_start_time );
    } else {
	// negedge first
	//m_next_negedge_event.notify_internal( m_start_time );
        m_next_negedge_event.notify( m_start_time );
    }
}

// Register enable CG signal/port
void sc_smart_clock::register_cg_enable(sc_signal_inout_if<bool>& enable) {
    cg_enables.push_back(&enable);
}

// Enable/disable clock 
void sc_smart_clock::control_action()
{
    bool a = false;
    for (auto&& s : cg_enables) {
        a = a || s->read();
    }
    m_clock_enable = a;
    m_next_negedge_event.notify( m_negedge_time );
    //std::cout << sc_time_stamp() << "," << sc_delta_count() << ", control_action " << a << std::endl;
}

#if ( defined(_MSC_VER) && _MSC_VER < 1300 ) //VC++6.0 doesn't support sc_spawn with functor.
#   define sc_smart_clock_posedge_callback(ptr) sc_smart_clock_posedge_callback

#   define sc_smart_clock_negedge_callback(ptr) sc_smart_clock_negedge_callback
 
#   define sc_spawn(a,b,c) { \
        sc_process_handle result(new sc_spawn_object<a>(a(this),b,c)); \
    }
#endif // ( defined(_MSC_VER) && _MSC_VER < 1300 )

void sc_smart_clock::before_end_of_elaboration()
{
    std::string gen_base;
    sc_spawn_options posedge_options;	// Options for posedge process.
    sc_spawn_options negedge_options;	// Options for negedge process.
            
    posedge_options.spawn_method();
    posedge_options.dont_initialize();
    posedge_options.set_sensitivity(&m_next_posedge_event);
    gen_base = basename();
    gen_base += "_posedge_action";
    sc_spawn(sc_smart_clock_posedge_callback(this),
             sc_gen_unique_name( gen_base.c_str() ), &posedge_options);

    negedge_options.spawn_method();
    negedge_options.dont_initialize();
    negedge_options.set_sensitivity(&m_next_negedge_event);
    gen_base = basename();
    gen_base += "_negedge_action";
    sc_spawn( sc_smart_clock_negedge_callback(this),
              sc_gen_unique_name( gen_base.c_str() ), &negedge_options );
    
    // Clock enable method
    sc_spawn_options enable_options;
    enable_options.spawn_method();
    enable_options.dont_initialize();
    enable_options.set_sensitivity(&m_check_enable_start);
    for (auto&& e : cg_enables) {
        enable_options.set_sensitivity(&(e->value_changed_event()));
    }
    gen_base = basename();
    gen_base += "_enable_action";
    sc_spawn( sc_smart_clock_control_callback(this),
              sc_gen_unique_name( gen_base.c_str() ), &enable_options );
}

//clear VC++6.0 macros
#undef sc_smart_clock_posedge_callback
#undef sc_smart_clock_negedge_callback
#undef sc_spawn


// destructor (does nothing)
sc_smart_clock::~sc_smart_clock()
{}

void sc_smart_clock::register_port( sc_port_base& /*port*/, const char* if_typename_ )
{
    std::string nm( if_typename_ );
    if( nm == typeid( sc_signal_inout_if<bool> ).name() ) {
	    SC_REPORT_ERROR(SC_ID_ATTEMPT_TO_BIND_CLOCK_TO_OUTPUT_, "");
    }
}

void
sc_smart_clock::write( const bool& /* value */ )
{
    SC_REPORT_ERROR(SC_ID_ATTEMPT_TO_WRITE_TO_CLOCK_, "");
}

// interface methods

// get the current time

const sc_time&
sc_smart_clock::time_stamp()
{
    return sc_time_stamp();
}


// error reporting

void
sc_smart_clock::report_error( const char* id, const char* add_msg ) const
{
    char msg[BUFSIZ];
    if( add_msg != 0 ) {
	std::sprintf( msg, "%s: clock '%s'", add_msg, name() );
    } else {
	std::sprintf( msg, "clock '%s'", name() );
    }
    SC_REPORT_ERROR( id, msg );
}


void
sc_smart_clock::init( const sc_time& period_,
		double         duty_cycle_,
		const sc_time& start_time_,
		bool           posedge_first_ )
{
    if( period_ == SC_ZERO_TIME ) {
	report_error( SC_ID_CLOCK_PERIOD_ZERO_,
		      "increase the period" );
    }
    m_period = period_;
    m_posedge_first = posedge_first_;

    if( duty_cycle_ <= 0.0 || duty_cycle_ >= 1.0 ) {
	m_duty_cycle = 0.5;
    } else {
	m_duty_cycle = duty_cycle_;
    }

    m_negedge_time = m_period * m_duty_cycle;
    m_posedge_time = m_period - m_negedge_time;

    if( m_negedge_time == SC_ZERO_TIME ) {
	report_error( SC_ID_CLOCK_HIGH_TIME_ZERO_,
		      "increase the period or increase the duty cycle" );
    }
    if( m_posedge_time == SC_ZERO_TIME ) {
	report_error( SC_ID_CLOCK_LOW_TIME_ZERO_,
		      "increase the period or decrease the duty cycle" );
    }

    if( posedge_first_ ) {
	this->m_cur_val = false;
	this->m_new_val = false;
    } else {
	this->m_cur_val = true;
	this->m_new_val = true;
    }

    m_start_time = start_time_;

}

} // namespace sc_core

