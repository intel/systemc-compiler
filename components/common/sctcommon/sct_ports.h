/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Single Source library. 
 * Input and output ports sct_in and sct_out to bind to sct_signal. 
 * sc_port specializations for Target and Initiator.
 * 
 * Author: Mikhail Moiseev, Leonid Azarenkov
 */

#ifndef SCT_PORT_TYPES_H
#define SCT_PORT_TYPES_H

#include "sct_ipc_if.h"
#include "sct_zero_width.h"
#include <systemc.h>
#include <vector>

namespace sct {
    
struct sct_sens_handle 
{
    sc_sensitive* s;
    sc_process_handle* p;
    sc_in_clk* c;

    sct_sens_handle(sc_sensitive* s_, sc_process_handle* p_, sc_in_clk* c_) : 
        s(s_), p(p_), c(c_) {}
};
    
/// Input port sct_in to bind to sct_signal
    
/// Cycle accurate implementation, inherits sc_in to compatibility with simulators
template <class T>
class sct_in<T, 0> : public sc_in<T>
{
  public:
    using base_type = sc_in<T>; 
    using this_type = sct_in<T, 0>;
    using signal_type = sct_signal<T, 0>;

    explicit sct_in() : base_type("sct_in") {}
    explicit sct_in(const char* name_) : base_type(name_) {}

    /// Store attached process handle and its module sensitivity list
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) {
        auto procKind = p->proc_kind();
        if (procKind == SC_METHOD_PROC_) {
            *s << *p << *dynamic_cast<base_type*>(this);
        } else {
            // Add nothing
        }
    }
};


/// Specialization for sct_zero_width, cycle accurate implementation
template<>
class sct_in<sc_dt::sct_zero_width, 0>: public sct_in_if<sc_dt::sct_zero_width>
{
  public:
    using this_type = sct_in<sc_dt::sct_zero_width, 0>;
    using signal_type = sct_signal<sc_dt::sct_zero_width, 0>;

    explicit sct_in() {}
    explicit sct_in(const char* name_) {}
    virtual ~sct_in() = default;

    sc_dt::sct_zero_width read() const override { return 0; }
    operator sc_dt::sct_zero_width () { return 0; }

    void operator () (signal_type& signal_) { }
    void bind(signal_type& signal_) { }

    void operator () (this_type& port_) { }
    void bind(this_type& port_) { }
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) override { }
    const char* kind() const { return "sct_in"; }
};


/// Approximate time implementation
template <class T>
class sct_in<T, 1> : public sct_in_if<T>
{
  public:   
    using IF = sct_in_if<T>;
    using this_type = sct_in<T, 1>;
    using signal_type = sct_signal<T, 1>;

    explicit sct_in() {}
    explicit sct_in(const char* name_) {}
    virtual ~sct_in() = default;

    T read() const override {
        return interface->read();
    }
        
    operator T () {
        return interface->read();
    }
    
  protected:
    /// Handle of the processes attached and their module sensitivity lists
    std::vector<sct_sens_handle>  sens_handle;
    
    /// Signal or port bound
    IF*     interface = nullptr;
    
  public:   
    /// Bind a signal with type IF to this port
    void operator () (signal_type& signal_) { 
        bind(signal_); 
    }

    void bind(signal_type& signal_) {
        if (!interface) {
            for (auto& entry : sens_handle) {
                signal_.addTo(entry.s, entry.p, entry.c);
            }
            sens_handle.clear();
            interface = &signal_;
        } else {
            std::cout << "Double bind sct_in to sct_signal " << signal_.name() << "\n";
            assert (false);
        } 
    }
    
    /// Bind a parent port with type IF to this port
    void operator () (this_type& port_) { 
        bind(port_); 
    }

    void bind(this_type& port_) {
        if (!interface) {
            for (auto& entry : sens_handle) {
                port_.addTo(entry.s, entry.p, entry.c);
            }
            sens_handle.clear();
            interface = &port_;
        } else {
            std::cout << "Double bind sct_in to parent sct_in port\n";
            assert (false);
        }
    }
    
    /// Store attached process handle and its module sensitivity list
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) override {
        if (interface) {
            interface->addTo(s, p, c);
        } else {
            sens_handle.emplace_back(s, p, c);
        }
    }
    
    const char* kind() const {
        return "sct_in";
    }
};


/// Specialization for sct_zero_width, approximate time implementation
template<>
class sct_in<sc_dt::sct_zero_width, 1>: public sct_in_if<sc_dt::sct_zero_width>
{
  public:
    using this_type = sct_in<sc_dt::sct_zero_width, 1>;
    using signal_type = sct_signal<sc_dt::sct_zero_width, 1>;

    explicit sct_in() {}
    explicit sct_in(const char* name_) {}
    virtual ~sct_in() = default;

    sc_dt::sct_zero_width read() const override { return 0; }
    operator sc_dt::sct_zero_width () { return 0; }

    void operator () (signal_type& signal_) { }
    void bind(signal_type& signal_) { }

    void operator () (this_type& port_) { }
    void bind(this_type& port_) { }
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) override { }
    const char* kind() const { return "sct_in"; }
};

//==============================================================================

/// Output port sc_out to bind to sct_signal

/// Cycle accurate implementation, inherits sc_out to compatibility with simulators
template <class T>
class sct_out<T, 0> : public sc_out<T>
{
  public:   
    using base_type = sc_out<T>; 
    using this_type = sct_out<T, 0>;
    using signal_type = sct_signal<T, 0>;

    explicit sct_out() : base_type("sct_out") {}
    explicit sct_out(const char* name_) : base_type(name_) {}

    this_type& operator = (const T& val) {
        base_type::write(val);
        return *this;
    }  
    
    this_type& operator = (const sc_signal_in_if<T>& other) {
        base_type::write(other.read());
        return *this;
    }
    
    this_type& operator = (const signal_type& other) {
        base_type::write(other.read());
        return *this;
    }
    
    /// Store attached process handle and its module sensitivity list
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) {
        auto procKind = p->proc_kind();
        if (procKind == SC_METHOD_PROC_) {
            *s << *p << *dynamic_cast<base_type*>(this);
        } else {
            // Add nothing
        }
    }
};


/// Specialization for sct_zero_width, cycle accurate implementation
template<>
class sct_out<sc_dt::sct_zero_width, 0> : public sct_inout_if<sc_dt::sct_zero_width>
{
  public:   
    using this_type = sct_out<sc_dt::sct_zero_width, 0>;
    using signal_type = sct_signal<sc_dt::sct_zero_width, 0>;

    explicit sct_out() { }
    explicit sct_out(const char* name_) { }
    virtual ~sct_out() = default;

    void write(const sc_dt::sct_zero_width& val) override { }
    this_type& operator = (const sc_dt::sct_zero_width& val) { return *this; }  
    this_type& operator = (const this_type& other) { return *this; }
    this_type& operator = (const signal_type& other) { return *this; }

    sc_dt::sct_zero_width read() const override { return 0; }
    operator sc_dt::sct_zero_width () { return 0; }

    void operator () (signal_type& signal_) { }
    void bind(signal_type& signal_) { }

    void operator () (this_type& port_) { }
    void bind(this_type& port_) { }
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) override { }
    const char* kind() const { return "sct_out"; }
};


/// Approximate time implementation
template <class T>
class sct_out<T, 1> : public sct_inout_if<T>
{
  public:   
    using IF = sct_inout_if<T>;
    using this_type = sct_out<T, 1>;
    using signal_type = sct_signal<T, 1>;

    explicit sct_out() {}
    explicit sct_out(const char* name) {}
    virtual ~sct_out() = default;

    void write(const T& val) override {
        interface->write(val);
    }
    
    this_type& operator = (const T& val) {
        write(val);
        return *this;
    }    
    
    this_type& operator = (const this_type& other) {
        write(other.read());
        return *this;
    }
    
    this_type& operator = (const signal_type& other) {
        write(other.read());
        return *this;
    }

    T read() const override {
        return interface->read();
    }

    operator T () {
        return interface->read();
    }

protected:
    /// Handle of the processes attached and their module sensitivity lists
    std::vector<sct_sens_handle>  sens_handle;
    
    /// Signal or port bound
    IF*     interface = nullptr;
    
  public:   
   /// Bind a signal with type IF to this port
    void operator () (signal_type& signal_) { 
        bind(signal_); 
    }

    void bind(signal_type& signal_) {
        if (!interface) {
            for (auto& entry : sens_handle) {
                signal_.addTo(entry.s, entry.p, entry.c);
            }
            sens_handle.clear();
            interface = &signal_;
        } else {
            std::cout << "Double bind sct_out to sct_signal " << signal_.name() << "\n";
            assert (false);
        } 
    }
    
    /// Bind a parent port with type IF to this port
    void operator () (this_type& port_) { 
        bind(port_); 
    }

    void bind(this_type& port_) {
        if (!interface) {
            for (auto& entry : sens_handle) {
                port_.addTo(entry.s, entry.p, entry.c);
            }
            sens_handle.clear();
            interface = &port_;
        } else {
            std::cout << "Double bind sct_out to parent sct_in port\n";
            assert (false);
        }
    }
      
    /// Store attached process handle and its module sensitivity list
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) override {
        if (interface) {
            interface->addTo(s, p, c);
        } else {
            sens_handle.emplace_back(s, p, c);
        }
    }    

    const char* kind() const {
        return "sct_out";
    }
};


/// Specialization for sct_zero_width, approximate time implementation
template<>
class sct_out<sc_dt::sct_zero_width, 1> : public sct_inout_if<sc_dt::sct_zero_width>
{
  public:   
    using this_type = sct_out<sc_dt::sct_zero_width, 1>;
    using signal_type = sct_signal<sc_dt::sct_zero_width, 1>;

    explicit sct_out() { }
    explicit sct_out(const char* name_) { }
    virtual ~sct_out() = default;

    void write(const sc_dt::sct_zero_width& val) override { }
    this_type& operator = (const sc_dt::sct_zero_width& val) { return *this; }  
    this_type& operator = (const this_type& other) { return *this; }
    this_type& operator = (const signal_type& other) { return *this; }

    sc_dt::sct_zero_width read() const override { return 0; }
    operator sc_dt::sct_zero_width () { return 0; }

    void operator () (signal_type& signal_) { }
    void bind(signal_type& signal_) { }

    void operator () (this_type& port_) { }
    void bind(this_type& port_) { }
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) override { }
    const char* kind() const { return "sct_out"; }
};

/// Type helper for sct_in<T>/sct_sinal<T> selection
template <typename T, bool is_signal>
struct sct_sig_in_sel;

template <typename TT>
struct sct_sig_in_sel<TT, true> {
    typedef sct_signal<TT> T;
};

template <typename TT>
struct sct_sig_in_sel<TT, false> {
    typedef sct_in<TT> T;
};

/// Type helper for sct_out<T>/sct_sinal<T> selection
template <typename T, bool is_signal>
struct sct_sig_out_sel;

template <typename TT>
struct sct_sig_out_sel<TT, true> {
    typedef sct_signal<TT> T;
};

template <typename TT>
struct sct_sig_out_sel<TT, false> {
    typedef sct_out<TT> T;
};


} // namespace sct

//==============================================================================

namespace sc_core {
    
/// Initiator port. Cycle accurate and approximate time implementation    
    
template<class T, class TRAITS, bool TLM_MODE>
class sc_port< sct::sct_initiator<T, TRAITS, TLM_MODE>, 1, SC_ONE_OR_MORE_BOUND> :
    public sc_port_b< sct::sct_initiator<T, TRAITS, TLM_MODE> >
{
  public:
    using IF = sct::sct_initiator<T, TRAITS, TLM_MODE>;
    using base_type = sc_port_b<IF>;
    using this_type = sc_port<IF,1,SC_ONE_OR_MORE_BOUND>;

    /// Bind an interface of type IF to this port
    void bind( IF& interface_ ) override {
        if (p) {
            interface_.addTo(s, p);
        }
        base_type::bind(interface_);
    }
    
    /// Bind a parent port with type IF to this port
    void bind( base_type& parent_ ) override {
        auto& parent_port = dynamic_cast<this_type&>(parent_);
        if (p) {
            if (parent_port.p) {
                ::std::cout << "\nParent port " << parent_.name() 
                            << " already attached to a process\n";
                assert (false);
            }
            parent_port.p = p; parent_port.s = s;
        }
        base_type::bind(parent_);
    }
    
    /// Store attached process handle and its module sensitivity list
    void addTo(sc_sensitive* s_, sc_process_handle* p_) {
        if (!p) {
            p = p_; s = s_;
        } else {
            ::std::cout << "Double attach port " << this->name() << " to a process\n";
            assert (false);
        }
    }    
    
  protected:    
    /// Handle of the put process attached and its module sensitivity list
    sc_process_handle*  p = nullptr;
    sc_sensitive* s = nullptr;
    
    void before_end_of_elaboration() override {
        p = nullptr;
        s = nullptr;
    }
    
  public:
    sc_port() : base_type(1, SC_ONE_OR_MORE_BOUND) {}
    explicit sc_port(const char* name) : base_type(name, 1, SC_ONE_OR_MORE_BOUND) {}
    virtual ~sc_port() {}
    virtual const char* kind() const { return "sc_port"; }
};    


/// Target port. Cycle accurate and approximate time implementation    
template<class T, class TRAITS, bool TLM_MODE>
class sc_port< sct::sct_target<T, TRAITS, TLM_MODE>, 1, SC_ONE_OR_MORE_BOUND> :
    public sc_port_b< sct::sct_target<T, TRAITS, TLM_MODE> >
{
  public:
    using IF = sct::sct_target<T, TRAITS, TLM_MODE>;
    using base_type = sc_port_b<IF>;
    using this_type = sc_port<IF,1,SC_ONE_OR_MORE_BOUND>;

    /// Bind an interface of type IF to this port
    void bind( IF& interface_ ) override {
        if (p) {
            interface_.addTo(s, p);
        }
        base_type::bind(interface_);
    }
    
    /// Bind a parent port with type IF to this port
    void bind( base_type& parent_ ) override {
        auto& parent_port = dynamic_cast<this_type&>(parent_);
        if (p) {
            if (parent_port.p) {
                ::std::cout << "Parent port " << parent_.name() 
                            << " already attached to a process\n";
                assert (false);
            }
            parent_port.p = p; parent_port.s = s;
        }
        base_type::bind(parent_);
    }
    
    /// Store attached process handle and its module sensitivity list
    void addTo(sc_sensitive* s_, sc_process_handle* p_) {
        if (!p) {
            p = p_; s = s_;
        } else {
            ::std::cout << "Double attach port " << this->name() << " to a process\n";
            assert (false);
        }
    }    
    
  protected:    
    /// Handle of the get process attached and its module sensitivity list
    sc_process_handle*  p = nullptr;
    sc_sensitive* s = nullptr;
    
    void before_end_of_elaboration() override {
        p = nullptr;
        s = nullptr;
    }
    
  public:
    sc_port() : base_type(1, SC_ONE_OR_MORE_BOUND) {}
    explicit sc_port(const char* name) : base_type(name, 1, SC_ONE_OR_MORE_BOUND) {}
    virtual ~sc_port(){}
    virtual const char* kind() const { return "sc_port"; }
};    

//==============================================================================

template<class T, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, sct::sct_in<T, TLM_MODE>& in_port )
{
    auto* p = new sc_process_handle(sc_get_current_process_handle());
    in_port.addTo(&s, p, sct::sct_curr_clock);
    return s;
}

template<class T, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, sct::sct_out<T, TLM_MODE>& out_port )
{
    auto* p = new sc_process_handle(sc_get_current_process_handle());
    out_port.addTo(&s, p, sct::sct_curr_clock);
    return s;
}

/// sc_port<sct_initiator> used by ICSC to have sct_initiator signals sensitive list
template<class T, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s,  
              sc_port< sct::sct_initiator<T, TRAITS, TLM_MODE> >& init_port )
{
    if (init_port.get_interface()) {
        init_port->addTo(s);
    } else {
        auto* p = new sc_process_handle(sc_get_current_process_handle());
        init_port.addTo(&s, p);
    }
    return s;
}

/// sc_port<sct_target> used by ICSC to have sct_target signals sensitive list
template<class T, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s,  
              sc_port< sct::sct_target<T, TRAITS, TLM_MODE> >& targ_port )
{
    if (targ_port.get_interface()) {
        targ_port->addTo(s);
    } else {
        auto* p = new sc_process_handle(sc_get_current_process_handle());
        targ_port.addTo(&s, p);
    }
    return s;
}

}  // namespace sc_core

#endif /* SCT_PORT_TYPES_H */

