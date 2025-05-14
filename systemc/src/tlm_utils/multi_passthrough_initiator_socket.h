/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/
#ifndef TLM_UTILS_MULTI_PASSTHROUGH_INITIATOR_SOCKET_H_INCLUDED_
#define TLM_UTILS_MULTI_PASSTHROUGH_INITIATOR_SOCKET_H_INCLUDED_

#include "multi_socket_bases.h"

#if defined(__clang__) || \
   (defined(__GNUC__) && ((__GNUC__ * 1000 + __GNUC_MINOR__) >= 4006))
// ignore warning about deliberately hidden "bind()" overloads
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

namespace tlm_utils {

/*
This class implements a trivial multi initiator socket.
The triviality refers to the fact that the socket does not
do blocking to non-blocking or non-blocking to blocking conversions.

It allows to connect multiple targets to this socket.
The user has to register callbacks for the bw interface methods
he likes to use. The callbacks are basically equal to the bw interface
methods but carry an additional integer that indicates to which
index of this socket the calling target is connected.
*/
template <typename MODULE,
          unsigned int BUSWIDTH = 32,
          typename TYPES = tlm::tlm_base_protocol_types,
          unsigned int N=0, sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
class multi_passthrough_initiator_socket
  : public multi_init_base< BUSWIDTH, TYPES, N, POL>
{
public:

  //typedefs
  //  tlm 2.0 types for nb_transport
  typedef typename TYPES::tlm_payload_type              transaction_type;
  typedef typename TYPES::tlm_phase_type                phase_type;
  typedef tlm::tlm_sync_enum                            sync_enum_type;

  //  typedefs to keep the fn ptr notations short
  typedef sync_enum_type (MODULE::*nb_cb)(int,
                                         transaction_type&,
                                         phase_type&,
                                         sc_core::sc_time&);
  typedef void (MODULE::*dmi_cb)(int, sc_dt::uint64, sc_dt::uint64);

  typedef multi_init_base<BUSWIDTH, TYPES, N, POL> base_type;

  typedef typename base_type::base_target_socket_type base_target_socket_type;

  static const char* default_name()
    { return sc_core::sc_gen_unique_name("multi_passthrough_initiator_socket"); }

  //CTOR
  explicit multi_passthrough_initiator_socket(const char* name = default_name())
      : base_type(name)
      , m_hierarch_bind(0)
      , m_beoe_disabled(false)
      , m_dummy(this,42)
  {
  }

  ~multi_passthrough_initiator_socket(){
    //clean up everything allocated by 'new'
    for (unsigned int i=0; i<m_binders.size(); i++) delete m_binders[i];
  }

  //register callback for nb transport of bw interface
  void register_nb_transport_bw(MODULE* mod,
                              sync_enum_type (MODULE::*cb)(int,
                                                           transaction_type&,
                                                           phase_type&,
                                                           sc_core::sc_time&))
  {
    //warn if there already is a callback
    if (m_nb_f.is_valid()){
      display_warning("NBTransport_bw callback already registered.");
      return;
    }

    //set the functor
    m_nb_f.set_function(mod, cb);
  }

  //register callback for dmi function of bw interface
  void register_invalidate_direct_mem_ptr(MODULE* mod,
                             void (MODULE::*cb)(int, sc_dt::uint64, sc_dt::uint64))
  {
    //warn if there already is a callback
    if (m_dmi_f.is_valid()){
      display_warning("InvalidateDMI callback already registered.");
      return;
    }

    //set the functor
    m_dmi_f.set_function(mod, cb);
  }

  //Override virtual functions of the tlm_initiator_socket:
  // this function is called whenever an sc_port (as part of a target socket)
  //  wants to bind to the export of the underlying tlm_initiator_socket
  //At this time a callback binder is created an returned to the sc_port
  // of the target socket, so that it binds to the callback binder
  virtual tlm::tlm_bw_transport_if<TYPES>& get_base_interface()
  {
    m_binders.push_back(new callback_binder_bw<TYPES>(this, m_binders.size()));
    return *m_binders[m_binders.size()-1];
  }

  // const overload not allowed for multi-sockets
  virtual const tlm::tlm_bw_transport_if<TYPES>& get_base_interface() const
  {
    display_error("'get_base_interface()' const not allowed for multi-sockets.");
    return base_type::get_base_interface();
  }

  //Override virtual functions of the tlm_initiator_socket:
  // this function is called whenever an sc_export (as part of a initiator socket)
  //  wants to bind to the export of the underlying tlm_initiator_socket
  //   i.e. a hierarchical bind takes place
  virtual sc_core::sc_export<tlm::tlm_bw_transport_if<TYPES> >& get_base_export()
  {
    if (!m_beoe_disabled) //we are not bound hierarchically
      base_type::m_export.bind(m_dummy);  //so we bind the dummy to avoid a SystemC error
    return base_type::get_base_export(); //and then return our own export so that the hierarchical binding is set up properly
  }

  virtual const sc_core::sc_export<tlm::tlm_bw_transport_if<TYPES> >& get_base_export() const
  {
    return base_type::get_base_export();
  }

  //bind against a target socket
  virtual void bind(base_target_socket_type& s)
  {
    //error if this socket is already bound hierarchically
    if (m_hierarch_bind) {
      display_error("Already hierarchically bound.");
      return;
    }

    base_type::bind(s); //satisfy systemC, leads to a call to get_base_interface()

    //try to cast the target socket into a fw interface
    sc_core::sc_export<tlm::tlm_fw_transport_if<TYPES> >* p_ex_s=dynamic_cast<sc_core::sc_export<tlm::tlm_fw_transport_if<TYPES> >*>(&s);
    if (!p_ex_s) {
      display_error("Multi socket not bound to tlm_socket.");
      return;
    }

    //try a cast into a multi sockets
    multi_to_multi_bind_base<TYPES>* test=dynamic_cast<multi_to_multi_bind_base<TYPES>*> (p_ex_s);
    if (test) //did we just do a multi-multi bind??
      //if that is the case the multi target socket must have just created a callback binder
      // which we want to get from it.
      //Moreover, we also just created one, which we will pass to it.
      m_sockets.push_back(test->get_last_binder(m_binders[m_binders.size()-1]));
    else{  // if not just bind normally
      sc_core::sc_export<tlm::tlm_fw_transport_if<TYPES> >& ex_s=*p_ex_s;
      m_sockets.push_back(&((tlm::tlm_fw_transport_if<TYPES>&)ex_s)); //store the interface we are bound against
    }
  }

  //operator notation for direct bind
  void operator() (base_target_socket_type& s)
  {
    bind(s);
  }

  //SystemC standard callback before end of elaboration
  void before_end_of_elaboration(){
    //if our export hasn't been bound yet (due to a hierarch binding)
    // we bind it now to avoid a SystemC error.
    //We must do that, because it is legal not to register a callback on this socket
    // as the user might only use b_transport
    if (!base_type::m_export.get_interface()){
      base_type::m_export.bind(m_dummy);
    }

    //'break' here if the socket was told not to do callback binding
    if (m_beoe_disabled) return;

    //get the callback binders of the top of the hierachical bind chain
    // NOTE: this could be the same socket if there is no hierachical bind
    std::vector<callback_binder_bw<TYPES>* >& binders=get_hierarch_bind()->get_binders();

    //get the interfaces bound to the top of the hierachical bind chain
    // NOTE: this could be the same socket if there is no hierachical bind
    m_used_sockets=get_hierarch_bind()->get_sockets();

    //register the callbacks of this socket with the callback binders
    // we just got from the top of the hierachical bind chain
    for (unsigned int i=0; i<binders.size(); i++) {
      binders[i]->set_callbacks(m_nb_f, m_dmi_f);
    }
  }

  //
  // Bind multi initiator socket to multi initiator socket (hierarchical bind)
  //
  virtual void bind(base_type& s)
  {
    if (m_binders.size()) {
      //a multi socket is either bound hierarchically or directly
      display_error("Socket already directly bound.");
      return;
    }
    if (m_hierarch_bind){
      display_warning("Socket already bound hierarchically. Bind attempt ignored.");
      return;
    }

    //remember to which socket we are hierarchically bound and disable it,
    // so that it won't try to register callbacks itself
    s.disable_cb_bind();
    m_hierarch_bind=&s;
    base_type::bind(s); //satisfy SystemC
  }

  //operator notation for hierarchical bind
  void operator() (base_type& s)
  {
    bind(s);
  }

  //get access to sub port
  tlm::tlm_fw_transport_if<TYPES>* operator[](int i){return m_used_sockets[i];}

  //get the number of bound targets
  // NOTE: this is only valid at end of elaboration!
  unsigned int size() {return get_hierarch_bind()->get_sockets().size();}

protected:
  using base_type::display_warning;
  using base_type::display_error;

  //implementation of base class interface
  base_type* get_hierarch_bind(){if (m_hierarch_bind) return m_hierarch_bind->get_hierarch_bind(); else return this;}
  void disable_cb_bind(){ m_beoe_disabled=true;}
  std::vector<callback_binder_bw<TYPES>* >& get_binders(){return m_binders;}
  std::vector<tlm::tlm_fw_transport_if<TYPES>*>& get_sockets(){return m_sockets;}
  //vector of connected sockets
  std::vector<tlm::tlm_fw_transport_if<TYPES>*> m_sockets;
  std::vector<tlm::tlm_fw_transport_if<TYPES>*> m_used_sockets;
  //vector of binders that convert untagged interface into tagged interface
  std::vector<callback_binder_bw<TYPES>*> m_binders;

  base_type*  m_hierarch_bind; //pointer to hierarchical bound multi port
  bool m_beoe_disabled;  // bool that remembers whether this socket shall bind callbacks or not
  callback_binder_bw<TYPES> m_dummy; //a callback binder that is bound to the underlying export
                                     // in case there was no real bind

  //callbacks as functors
  // (allows to pass the callback to another socket that does not know the type of the module that owns
  //  the callbacks)
  typename callback_binder_bw<TYPES>::nb_func_type  m_nb_f;
  typename callback_binder_bw<TYPES>::dmi_func_type m_dmi_f;
};

template <typename MODULE,
          unsigned int BUSWIDTH = 32,
          typename TYPES = tlm::tlm_base_protocol_types,
          unsigned int N=0>
class multi_passthrough_initiator_socket_optional
  : public multi_passthrough_initiator_socket<MODULE,BUSWIDTH,TYPES,N,sc_core::SC_ZERO_OR_MORE_BOUND>
{
  typedef multi_passthrough_initiator_socket<MODULE,BUSWIDTH,TYPES,N,sc_core::SC_ZERO_OR_MORE_BOUND> socket_b;
public:
  multi_passthrough_initiator_socket_optional() : socket_b() {}
  explicit multi_passthrough_initiator_socket_optional(const char* name) : socket_b(name) {}
};

} // namespace tlm_utils

#if defined(__clang__) || \
   (defined(__GNUC__) && ((__GNUC__ * 1000 + __GNUC_MINOR__) >= 4006))
#pragma GCC diagnostic pop
#endif

#endif // TLM_UTILS_MULTI_PASSTHROUGH_INITIATOR_SOCKET_H_INCLUDED_
