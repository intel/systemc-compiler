# Introduction 

SingleSource library consists of communication channels which implements SC interfaces. In the following figure the SingleSource channels are presented (dot outline components not open sourced yet).

Target and Initiator are intended to connect two SC modules with 1:1 connection. Multi-target and Multi-initiator modules provides 1:N connection to connect multiple SC modules. FIFO is intended to connect two processes in the same module or to serve as a buffer for one process.  
On-die port represents any external port to connect SC design to other IPs or fabric. Memory represents any kinds of on-chip SRAM, RF or ROM memory. Register is used to add state for METHOD process. The common use cases of the modules are given in the picture below.

The Single Source modules work in two modes: cycle accurate (RTL) and approximate time (TLM). 
Cycle accurate _RTL mode_ intended for hardware synthesis. In RTL mode the modules provide cycle accurate simulation. RTL mode is used if precise simulation results are required.
Approximate time _TLM (Transaction Level Modelling) mode_ provide fast simulation, intended for virtual prototyping. In TLM mode the modules provide approximate time simulation. TLM mode is implemented to be similar to RTL mode until that reduce simulation speed. That practically means exact time of an event can differ in TLM and RTL modes.

## Library files
SingleSource library is header only library -- sct_common.h needs to be included.

Most important files are:
* sct_common.h -- includes of all library headers and adds using namespace sct
* sct_ipc_if.h -- contains interfaces, general template types and defines
* sct_initiator.h -- initiator module, ```sct_initiator```
* sct_target.h -- target and combinational target modules, ```sct_target```
* sct_multi_initiator.h -- initiator to connect multiple targets, ```sct_multi_target```
* sct_multi_target.h -- target to connect multiple initiators, ```sct_multi_initiator```
* sct_signal.h -- signal implementation, ```sct_signal```
* sct_comb_signal.h -- signal combinationally assigned in thread process to avoid cycle delay, ```sct_comb_signal```
* sct_ports.h -- input and output ports, sc_port for target and initiator, ```sct_in```, ```sct_out```
* sct_cdc_fifo.h -- Dual-clock (Clock Domain Crossing) FIFO channel, ```sct_cdc_fifo```
* sct_fifo.h -- FIFO channel, ```sct_fifo```
sct_buffer.h -- buffer channel, which is a fast FIFO to use in clocked process(es) only, ```sct_buffer```
* sct_pipe.h -- pipeline register, ```sct_pipe```
* sct_register.h -- register to store METHOD state, ```sct_register```
* sct_clock.h -- clock with enable/disable, ```sct_clock```
* sct_clk_gate_cell.h -- clock gate and clock gate signal, ```sct_clk_signal```, ```sct_clk_gate_cell ```
* sct_ff_sync_cell.h -- Flip-Flop synchronizer, ```sct_ff_sync_cell```
* sct_sel_type.h -- integer types ```sct_int``` and ```sct_uint```
* sct_static_log.h -- static logarithm functions
* sct_utils.h -- utility functions

## Library defines
```SCT_TLM_MODE``` could be provided as compile definition: if ```SCT_TLM_MODE``` defined TLM mode is used, RTL mode is used otherwise.

There are multiple options for clock/reset levels specified by ```SCT_CMN_TRAITS``` with one of six following options:
  * ```SCT_POSEDGE_NEGRESET``` -- positive clock edge, negative reset level 
  * ```SCT_POSEDGE_POSRESET``` -- positive clock edge, positive reset level
  * ```SCT_NEGEDGE_NEGRESET``` -- negative clock edge, negative reset level
  * ```SCT_NEGEDGE_POSRESET``` -- negative clock edge, positive reset level
  * ```SCT_BOTHEDGE_NEGRESET``` -- both clock edges, negative reset level
  * ```SCT_BOTHEDGE_POSRESET``` -- both clock edges, positive reset level

Usually, positive clock edge and negative reset level are used. That is provided by define ```SCT_CMN_TRAITS```: 
```cpp
#ifndef SCT_CMN_TRAITS
  #define SCT_CMN_TRAITS SCT_POSEDGE_NEGRESET
#endif
```
If other clock edge/reset levels required, ```SCT_CMN_TRAITS``` value should be provided as compile definition.

There is an ```CMakeLists.txt``` example where ```sct_def_traits``` target has definitions for TLM mode, negative clock edge and positive reset level:
```cmake 
add_executable(sct_def_traits sc_main.cpp)
target_compile_definitions(sct_def_traits PUBLIC -DSCT_TLM_MODE)
target_compile_definitions(sct_def_traits PUBLIC -DSCT_CMN_TRAITS=SCT_NEGEDGE_POSRESET)
```

```DEBUG_SYSTEMC``` is used to enable additional checks, provide ```trace()``` functions and debug signal for waveforms viewers. That is specially important for debugging waveforms of channels implemented as primitive channel inheritor (```sct_buffer```). 


## Simulation modes 

SystemC design with SingleSource channels can be run in the following modes.

| Purpose | Defines required | Comment |
| ---------- | --------- | ------- |
| Cycle accurate simulation | - | Any C++ IDE |
| Cycle accurate simulation with waveforms | ```#define DEBUG_SYSTEMC``` | SystemC traces and vendor simulation tools |
| High-level synthesis with ICSC | ```#define __SC_TOOL__``` | The define provided by ```svc_target()``` |
| Approximately timed simulation | ```#define SCT_TLM_MODE``` | Used to speed simulation |
| Loosely timed simulation | ```#define SCT_TLM_MODE``` | Set template parameter ```TLM_MODE = SCT_CMN_LT_MOD``` for the specified channels |


## Library interfaces

The interfaces contain non-blocking functions except ```b_put``` and ```b_get``` which are may-blocking.

| Interface  | Functions | Comment |
| ---------- | --------- | ------- |
| sct_put_if<T>  | ```bool ready()``` | Return true if the channel is ready to put request |
|                | ```void reset_put()``` | Reset this channel |
|                | ```void clear_put()``` | Clear (remove) request put in this cycle |
|                | ```bool put(const T& data)``` | Non-blocking put request into the channel if it is ready, return ready to request |
|                | ```bool put(const T& data, sc_uint<N> mask)``` | Non-blocking put request into the channel if it is ready, ```mask``` used to enable/disable put or choose targets in multi-cast put, return ready to request |
|                | ```void b_put(const T& data)``` | May-blocking put request, could be used in sequential process only  |
|                | ```void addTo(sc_sensitive& s)``` | Add put related signals to process sensitivity | 
|                | ```void addTo(sc_sensitive* s, sc_process_handle* p)``` | Add put related signals to process sensitivity | 
| sct_get_if<T>  | ```bool request()``` | Return true if the channel has request to get |
|                | ```void reset_get()``` | Reset this channel |
|                | ```void clear_get()``` | Clear (return back) request got in this cycle |
|                | ```T peek()``` | Peek request, return current request data, if no request last data returned |
|                | ```T get()``` | Non-blocking get request and remove it from the channel, return current request data, if no request last data returned |
|                | ```bool get(T& data, bool enable)``` | Non-blocking get request and remove it from the channel if ```enable``` is true, return true if there is a request and ```enable``` is true | 
|                | ```T b_get()```  | May-blocking get request, could be used in sequential process only |
|                | ```void addTo(sc_sensitive& s)``` | Add get related signals to process sensitivity | 
|                | ```void addTo(sc_sensitive* s, sc_process_handle* p)``` | Add get related signals to process sensitivity | 
|                | ```void addPeekTo(sc_sensitive& s)``` | Add peek related signal to process sensitivity | 
| sct_fifo_if<T> | inherits ```sct_put_if<T>``` and ```sct_get_if<T>``` interfaces |  |
|                | ```unsigned size()``` | This channel size, specified as template parameter |
|                | ```unsigned elem_num()``` | Number of elements in the channel, value updated last clock edge for METHOD, last DC for THREAD |
|                | ```bool almost_full(unsigned N)``` | Return true if the channel has (LENGTH-N) elements or more, value updated last clock edge for METHOD, last DC for THREAD |
|                | ```bool almost_empty(unsigned N)``` | Return true if the channel has N elements or less, value updated last clock edge for METHOD, last DC for THREAD |
|                | ```void clk_nrst(sc_in<bool>& clk_in, sc_in<bool>& nrst_in)``` | Bind clock and reset to the channel |
|                | ```void addTo(sc_sensitive& s)``` | Add put and get related signal to process sensitivity |
|                | ```void addToPut(sc_sensitive& s)``` | Add put related signals to process sensitivity |
|                | ```void addToGet(sc_sensitive& s)``` | Add get related signals to process sensitivity |
| sct_in_if<T>   | ```const T& read()``` | Read from the signal/register |
|                | ```void addTo(sc_sensitive* s, sc_process_handle* p)``` | Add signals to process sensitivity | 
| sct_inout_if<T>| ```const T& read()``` | Read from the signal/register |
|                | ```void write(const T& val)``` | Write to the signal/register | 
|                | ```void addTo(sc_sensitive* s, sc_process_handle* p)``` | Add signals to process sensitivity | 

Functions ```addTo```, ```addToPut```, ```addToGet``` and ```addPeekTo``` are used to add the channel to process sensitivity list. For target and initiator instead ```addTo``` operator ```<<``` can be used. For FIFO instead ```addToPut``` and ```addToGet``` operator ```<< fifo.PUT```, ```<< fifo.GET```, and ```<< fifo.PEEK``` can be used.



# Target and Initiator

Target and Initiator are channels intended to connect two user defined modules. Initiator implements ```sct_put_if``` interface and could be used in one METHOD or THREAD process to put requests. Target implements ```sct_get_if``` interface and could be used in one METHOD or THREAD process to get requests which put by the connected Initiator.

## Target and initiator instantiation and bind

To connect two modules, Target placed in one modules, Initiator in another one. Target and Initiator should be connected to clock and reset with ```clk_nrst()``` function. Target and Initiator are connected to each other with method ```bind()```, called in their common parent module constructor. Both Target and Initiator have method ```bind()```, any of them can be called.  

```cpp
struct Producer : public sc_module {
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sct_initiator<T>    init{"init"};
    explicit Producer (const sc_module_name& name) : sc_module(name) {
        init.clk_nrst(clk, nrst);
    } 
}
struct Consumer : public sc_module {
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sct_target<T>       targ{"targ"};
    explicit Consumer (const sc_module_name& name) : sc_module(name) {
        targ.clk_nrst(clk, nrst);
    } 
}
struct Top: public sc_module {
    Producer prod{"prod"};
    Consumer cons{"cons"};
    explicit Top(const sc_module_name& name) : sc_module(name) {
        prod.clk(clk); prod.nrst(nrst);
        cons.clk(clk); cons.nrst(nrst);
        // Call bind() method of initiator or bind() method of target
        prod.init.bind(cons.targ);  
    }
}
```

Target and Initiator have the same template parameters:
```cpp
template<
    class T,                            // Payload data type 
    class TRAITS = SCT_CMN_TRAITS,  // Clock edge and reset level traits
    bool TLM_MODE = SCT_CMN_TLM_MODE>   // RTL (0) or TLM (1) mode
class sct_initiator {};

template<
    class T,                            // Payload data type 
    class TRAITS = SCT_CMN_TRAITS,  // Clock edge and reset level traits
    bool TLM_MODE = SCT_CMN_TLM_MODE>   // RTL (0) or TLM (1) mode
class sct_target {};
```

Target and Initiator constructor parameters:
```cpp
sct_target(const sc_module_name& name,      // Module name -- same as instance variable name
           bool sync_ = 0,                  // Is register required to pipeline request 
           bool always_ready_ = 0);         // Is always ready to get request

sct_initiator(const sc_module_name& name,   // Module name -- same as instance variable name  
           bool sync_ = 0);                 // Is register required to pipeline request  
```
That is enough to set```sync_ = 1``` for Target or for Initiator to have register added.

## Target and initiator usage

Target and initiator can be used in SystemC method process. The method process should be created with ```SC_METHOD``` or ```SCT_METHOD``` macro in the module constructor. The method process should have sensitivity list with all the targets/initiators accessed in the process function.

```cpp
// Initiator and target in method process example
struct Producer : public sc_module {
    sct_initiator<T>         init{"init"};
    explicit Producer (const sc_module_name& name) : sc_module(name) {
       SC_METHOD(initProc); 
       sensitive << init;
    } 
    void initProc {
       // Put data into init
    }
}

struct Consumer : public sc_module {
    sct_target<T>       targ{"targ"};   
    explicit Consumer (const sc_module_name& name) : sc_module(name) {
       SC_METHOD(targProc); 
       sensitive << targ;
    } 
    void targProc{
       // Get data from targ
    }
}
```

Target and initiator can be used in clocked thread process. Clocked thread process should be created with ```SC_THREAD``` or ```SCT_THREAD``` macro, but not with ```SC_CTHREAD```. The thread process should have sensitivity list with all the targets/initiators accessed in the process function as for method process. If the thread process has reset signal, it should have the reset specification with ```async_reset_signal_is``` or/and ```sync_reset_signal_is```.

```cpp
// Initiator and target in thread process example
struct Producer : public sc_module {
    sct_initiator<T>         init{"init"};
    explicit Producer (const sc_module_name& name) : sc_module(name) {
       SC_THREAD(initProc); 
       sensitive << init;
       async_reset_signal_is(nrst, 0);
    } 
    void initProc {
       // Reset init to set default values
       wait();
       while(true) {
          // Put data into init 
          wait();
       }
    }
}

struct Consumer : public sc_module {
    sct_target<T>       targ{"targ"};   
    explicit Consumer (const sc_module_name& name) : sc_module(name) {
       SC_THREAD(targProc); 
       sensitive << targ;
       async_reset_signal_is(nrst, 0);
    } 
    void targProc{
       // Reset init to set default values
       wait();
       while(true) {
          // Get data from targ
          wait();
       }
    }
}
```

There are three kinds of connections which could be organized: 
  * Combinational, 
  * Buffered,
  * Buffered with FIFO. 

## Combinational connection

In combinational connection request part of connection contains ```core_req``` and ```core_data``` signals, which could be used directly or through the pipelining register (specified with second parameter of Target/Initiator constructor). There is no back-pressure signal, so Target process should be always ready to get request. Initiator process does not need to check ready to put request (method ```ready()``` always returns true).

Combinational connection is provided with last parameter of ```sct_target<>``` constructor or with using special target class ```sct_comb_target<>```.

In combinational connection put data into Initiator can be done without checking if the Initiator is ready.

```cpp
// Initiator and always ready target in method process example
struct Producer : public sc_module {
    sct_initiator<T>         init{"init"};
    explicit Producer (const sc_module_name& name) : sc_module(name) {
       SC_METHOD(initProc); sensitive << init;
    } 
    void initProc {
       T val = getSomeValue();     // Put at every path, reset is not required 
       init.put(val);              // Do not check ready() as connected Target is always ready
    }
}

struct Consumer : public sc_module {
    // Combinational target
    sct_comb_target<T>       targ{"targ"};   
    explicit Consumer (const sc_module_name& name) : sc_module(name) {
       SC_METHOD(targProc); sensitive << targ;
    } 
    void targProc{
       T val;
       if (targ.get(val)) {     // Get at every path, reset is not required
           doSomething(val);        
       }
    }
}
```

In thread process it needs to reset Initiator and Target in the reset section. 

```cpp
// Initiator and always ready target in thread process example
struct Producer : public sc_module {
    sct_initiator<T>         init{"init"};
    explicit Producer (const sc_module_name& name) : sc_module(name) {
       SC_THREAD(initProc); sensitive << init;
       async_reset_signal_is(nrst, 0);
    } 
    void initProc {
       init.reset_put();              // Reset is required in thread process
       wait();
       while(true) {
          T val = getSomeValue();     // Put every cycle
          init.put(val);              // Do not check ready() as connected Target is always ready
          wait();
       }
    }
}

struct Consumer : public sc_module {
    sct_comb_target<T>       targ{"targ"};   
    explicit Consumer (const sc_module_name& name) : sc_module(name) {
       SC_THREAD(targProc); sensitive << targ;
       async_reset_signal_is(nrst, 0);
    } 
    void targProc{
       targ.reset_get();              // Reset is required in thread process
       wait();
       while(true) {
          if (targ.request()) {         
              doSomething(targ.get());
          }
          wait();
       }
    }
}
```

Using Target and Initiator in method and thread process looks very similar. In the next sections examples using method and thread process will be mixed.

## Buffered connection

In buffered connection ```core_ready``` signal is used as backpressure when Target is not ready to get request. This connection called buffered as it has the buffer register inside Target or Initiator to store one request if Target is not ready. THis kind of connection is the most common and used as default one.

Request part of the connection contains ```core_req``` and ```core_data``` signals, which could be used directly or through the pipelining register (specified with second parameter of Target/Initiator constructor). The pipelining register is additional to the buffer register. Response part contains ```core_ready``` signal which is passed through register to avoid combinational loop. If target process is method this register explicitly added, if it is thread this register is implicitly provided by the process.

```cpp
struct Producer : public sc_module {
    sct_initiator<T>         init{"init"};
    explicit Producer (const sc_module_name& name) : sc_module(name) {
       SC_METHOD(initProc); sensitive << init;
    } 
    void initProc {
       init.reset_put();            // Reset required as put is done at some path only  
       if (init.ready()) {          // Check ready required as target could be not ready 
          init.put(getSomeValue());
       }
    }
}

struct Consumer : public sc_module {
    sct_target<T>       targ{"targ"};
    explicit Consumer (const sc_module_name& name) : sc_module(name) {
       SC_THREAD(targProc); sensitive << targ;
       async_reset_signal_is(nrst, 0);
    } 
    void targProc {
       targ.reset_get();
       wait();
       while(true) {
          if (targ.request()) {      
              doSomething(targ.get()); 
          }
          wait(); 
       }
    }
}
```

## Buffered connection with FIFO

The buffered connection with FIFO provides additional buffer to store requests until their processed by the target process.
FIFO can be added to Target with ```add_fifo()``` method:
```cpp
template<unsigned LENGTH>                 // FIFO size (maximal number of elements)
void add_fifo(bool sync_valid = 0,        // Is register required to pipeline core_req and core_data
              bool sync_ready = 0,        // Is register required to pipeline core_ready 
              bool init_buffer = 0);      // Initialize all the elements with zeros 
                                          // First element to get is always initialized to zero 
```

```cpp
template<class T>
struct A : public sc_module {
    sct_target<T>       run{"run"}; 
    explicit A(const sc_module_name& name) : sc_module(name) {
        run.clk_nrst(clk, nrst);
        run.template add_fifo<2>(1, 1);  // Add FIFO with 2 element and registers in request/response
    }
}
```


## Multi-initiator connections

Multi-initiator connects multiple targets. Multi-initiator provides combinational and combinational with register connections only. 
Number of connected targets is specified with second template parameter.
```cpp
template<
    class T, 
    unsigned N,       // Number of connected targets
    ...>
class sct_multi_initiator {};
```

Multi-cast put request allows to choose specific targets with ```mask``` parameter, each bit in ```mask``` correspondent to a target connected.

```cpp
bool put(const T& data, 
         sc_uint<N> mask);     // Mask parameter to specify Targets for multi-cast request
```

## Multi-target connections

Multi-target connects multiple initiators. Multi-target provides combinational and combinational with register connections only. 
Number of connected initiatorsis specified with second template parameter.
```cpp
template<
    class T, 
    unsigned N,       // Number of connected initiators
    ...>
class sct_multi_target {};
```

Only one request from one of the connected initiators is allowed. If there is more than one request, error is reported in RTL mode. In TLM mode only one request in DC is supported, any extra requests are missed that leads to incorrect simulation results. 


# Signal and ports

Signal ```sct_signal``` can be used for inter-process communication between processes in the same module. For communication between processes in different modules input/output ports are used together with signal. 

Signal and output port ```sct_out``` implement ```sct_inout_if```, and can be written by one process. Signal, input and output ports implement ```sct_in_if```, and can be read by one or mode processes.

```cpp
template<
    class T, bool ENABLE_EVENT, bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_signal {};
template<
    class T, bool ENABLE_EVENT, bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_in {};   // Input port
template<
    class T, bool ENABLE_EVENT, bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_out {};  // Output port
```

Using signal and input/output ports in thread process requires to have clock/reset for these channels which provided with ```SCT_THREAD``` macro:
```cpp
SCT_THREAD(proc, clk, rst);  /// Used if the process sensitive to signals/ports only  
SCT_THREAD(proc, clk);       /// Used if the process sensitive to signals/ports and other channels
``` 

In this example ```sigThread``` sensitive to signals only:
```cpp
sct_signal<T>   s{"s"};
MyModule(const sc_module_name& name) : sc_module(name) {  
   SCT_THREAD(sigThread, clk, nrst);    // Clock edge/reset level taken from SCT_CMN_TRAITS
   sensitive << s;                      // Only signal `s` is read inside the process
   async_reset_signal_is(nrst, 0);
}
```

```sc_vector``` of ```sct_signal```, ```sct_in``` and ```sct_out``` supported. Binding of whole vector to another vector is supported.
```cpp
class A : public sc_module {
    sc_vector<sct_out<T>>      resp{"resp", 3};
};
class Top {
    A   a{"a"};
    sc_vector<sct_signal<T>>   resp{"resp", 3};

    Top (const sc_module_name& name) : sc_module(name) {        
        a.resp(resp);   // All vector elements bound
    }
}
```


## Level-enable signal and ports

Level enable signal is often used by one process to make another process does some actions. The signal is usually boolean type. It can have a specific level (usually high) for one or several cycle that means some action to be done. Such behavior work well in cycle accurate mode, but for fast simulation mode requires event notification at every cycle. To support this event notification ```sct_signal``` and ```sct_in```/```sct_out``` have second template parameter ```ENABLE_EVENT```. Also special level-enable signal and ports of boolean type are introduced: ```sct_enable_signal```, ```sct_enable_in```, and ```sct_enable_out```. 
Level-enable singal in fast simulation mode notifies event to wake a sensitive thread process every clock cycle which is taken from the process. 


In the following example producer process send multiple sequenced event.  
```cpp
SC_MODULE(MyModule) {
    sct_target<unsigned>      SC_NAMED(targ);
    SC_CTOR(MyModule) {
        SCT_THREAD(producer, clk, nrst); 
        sensitive << enable << targ;
        async_reset_signal_is(nrst, 0);
        SCT_THREAD(consumer, clk, nrst); 
        sensitive << enable;
        async_reset_signal_is(nrst, 0);
    }
    sct_enable_signal   SC_NAMED(enable);
    void producer() {
        targ.reset_get(); enable = 0;
        wait();
        while (true) {
            unsigned N = targ.b_get();
            enable = 1;
            while (N--) wait();
            enable = 0;
            wait();
        }
    }
    void consumer() {
        wait();
        while (true) {
            if (enable) {...};  // Do something
            wait();
}}}
```

Combinational method process cannot be sensitive for such a signal or port. If it needs to connect Thread-to-Method only general signal should be used. But combinational method can drive level enable signal, so Method-to-Thread interaction can used such a signal.

# FIFO

The FIFO can be used for inter-process communication between processes in the same module and for storing requests inside one process. Also the FIFO could be used inside of Target as an extended buffer. 

The FIFO implements ```sct_fifo_if```. FIFO has size template parameter which is a positive number.
```cpp
template<
    class T, 
    unsigned LENGTH,                    // Size (maximal number of elements)
    class TRAITS = SCT_CMN_TRAITS,      // Clock edge and reset level traits
    bool TLM_MODE = SCT_CMN_TLM_MODE>   // RTL (0) or TLM (1) mode
>
class sct_fifo {};
```

The FIFO can have combinational or registered request (```core_req``` and ```core_data```) and response (```core_ready```) kind which specified in constructor parameters. 
```cpp
sct_fifo(const sc_module_name& name, 
         bool sync_valid = 0,       // Request path has synchronous register 
         bool sync_ready = 0,       // Response path has synchronous register  
         bool use_elem_num = 0,     // Element number/Almost full or empty used 
         bool init_buffer = 0)      // Initialize all buffer elements with zeros in reset
                                    // First element to get is always initialized to zero 
```
Using synchronous register in request path (```sync_valid```) not allowed if put process is sequential thread or method. Using synchronous register in response path (```sync_ready```) not allowed if get process is sequential thread or method. That is required to have equivalent behavior in the generated SV code. 

### Minimal FIFO size required

Minimal FIFO size to provide full throughput depends on process types and request/response kind. In the table below minimal required FIFO sizes to provide full throughput are given. 

Using FIFO in method process(es) with both ```sync_valid``` and ```sync_ready``` set to ```0``` is prohibited as that results in combinational loop. 
Using FIFO in one method process is allowed with ```sync_valid``` and ```sync_ready``` both set to ```1``` only. If ```sync_valid``` or ```sync_ready``` set to ```0```, such FIFO can be used in two different method processes.

| Put process | Get process | sync_valid | sync_ready |  Minimal FIFO size |
| --------- | ------- | -------- | ---------- | ------- |
| method | method | 0 | 0 | prohibited |
| method | method | 0 | 1 | 1, two processes |
| method | method | 1 | 0 | 1, two processes |
| method | method | 1 | 1 | 2 |
| method | thread | 0 | 0 | 1 |
| method | thread | 1 | 0 | 2 |
| method | thread | 0 | 1 | not supported |
| method | thread | 1 | 1 | not supported |
| thread | method | 0 | 0 | 1 |
| thread | method | 0 | 1 | 2 |
| thread | method | 1 | 0 | not supported |
| thread | method | 1 | 1 | not supported |
| thread | thread | 0 | 0 | 2 |
| thread | thread | 0 | 1 | not supported |
| thread | thread | 1 | 0 | not supported |
| thread | thread | 1 | 1 | not supported |

### Using FIFO for inter-process communication

FIFO could be used for processes communication. FIFO allow only one writer and one reader process, but multiple process can do peek from the FIFO.
Normally, FIFO used between two processes.

FIFO used between two combinational method processes must have ```sync_valid``` or ```sync_ready``` parameter set to 1. 

```cpp
struct Top : public sc_module {
    sct_fifo<T, 2>      fifo{"fifo", 0, 1};     // Pipelining register for response
    explicit Top(const sc_module_name& name) : sc_module(name) {
        fifo.clk_nrst(clk, nrst);
        SC_THREAD(producerProc); sensitive << fifo.PUT;   // Process puts to FIFO
        async_reset_signal_is(nrst, 0);
        SC_METHOD(consumerProc); sensitive << fifo.GET;   // Process gets from FIFO   
    } 
}

void producerProc() {
    fifo.reset_put();
    wait();
    while (true) {
       if (fifo.ready()) {           // If FIFO is ready put next value
          fifo.put(getSomeVal());
       }
       wait();
    }
}
void consumerProc() {
    fifo.reset_get();
    T val;
    if (fifo.get(val)) {
       doSomething(val);
    }
}
```

### One process stores requests in FIFO

One process stores requests in FIFO example.

```cpp
struct Top : public sc_module {
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sct_fifo<T, 5>      fifo{"fifo"};
    explicit Top(const sc_module_name& name) : sc_module(name) {
        fifo.clk_nrst(clk, nrst);
        SC_THREAD(storeProc); sensitive << fifo;  // Process puts and gets to FIFO
        async_reset_signal_is(nrst, 0);
    }
}

void storeProc() {
    fifo.reset();
    wait();
    while (true) {
       if (fifo.ready()) {
          fifo.put(getSomeValue());
       }
       wait(); 
       if (fifo.request()) {
          doSomething(fifo.get());
       }
    }
}
```

# Buffer

Buffer is a channel kind of FIFO to be used in single or two sequential processes. Buffer implements ```sct_fifo_if``` interface, the same as FIFO.
Buffer differs from FIFO in higher simulation speed which is achieved by implementation it as primitive channel (inheritor of ```sc_prim_channel```). 
Buffer has one common implementation for cycle accurate and approximate time modes.
 
Buffer size can be 2 elements or more.
```cpp
template<
    class T, 
    unsigned LENGTH,                    // Size (maximal number of elements)
    class TRAITS = SCT_CMN_TRAITS       // Clock edge and reset level traits
>
class sct_buffer {};
```

Buffer constructor has the same parameters as FIFO has. Parameters ```sync_valid``` and ```sync_ready``` should be ```0 (false)```.

```cpp
sct_buffer(const sc_module_name& name, 
           bool sync_valid = 0,       // Request path has synchronous register 
           bool sync_ready = 0,       // Response path has synchronous register  
           bool use_elem_num = 0,     // Element number/Almost full or empty used 
           bool init_buffer = 0)      // Initialize all buffer elements with zeros in reset
                                      // First element to get is always initialized to zero 
```

Buffer can be used by one process to store data and for inter-process communication between two processes. ```peek()``` function of Buffer can be called from any process including combinational method process.


# Free index stack

Stack of free indices for resource allocation. It is initially filled with all indices ```0..N-1```, so after reset the stack is full.
Stack parameter ```LENGTH``` defines the number of available indices in the stack. Indices are popped from the stack when resources are allocated. Indices are pushed back to the stack when resources are freed.
Index stack can be used in one process which is ```SC_CTHREAD```/```SCT_THREAD``` only.

```cpp
template <
    unsigned LENGTH,               /// Stack size
    class TRAITS = SCT_CMN_TRAITS  /// Clock edge and reset level traits
>
class sct_index_stack {};
```

Index stack has ```reset()``` method which should be called in the process reset section.
It has two main methods ```pop``` and ```push``` to get a free index and to return an index back.
There are also additional methods to get current free index and check Index stack number of elements.

```cpp
/// Push index onto the stack (return to free index pool)
void push(const Idx_t & idx);
/// Pop index from the stack (get free index from the pool)
Idx_t pop();    
/// Get current free index at the stack top
Idx_t free_idx() const;
/// Check if stack is empty
bool empty() const;
/// Check if stack is full
bool full() const;
```

There is a simplified example of Index stack usage:
```cpp
sct_index_stack<N>   SC_NAMED(istack);
using Idx_t = sct_index_stack<N>::Idx_t;

void proc() {    
   istack.reset();
   wait();
        
   while (true) {
       if (!istack.empty()) {
           idx = istack.pop();
           ... // use the index to access a memory
       }            
       if (!istack.full()) {           
           Idx_t idx = ...;  // index to return back to the stack
           istack.push(idx); 
       }
       wait();
   }
```




# Shift register

Shift register is intendent to introduce multiple clock delay. It is implemented as a sequence of registers of the specified width. Output of a register is an input to another register to assign the value every clock.
Shift register can be used in one process which is ```SC_CTHREAD```/```SCT_THREAD``` only.

Shift register size can be 1 to 256 elements.
```cpp
template<
    class T, 
    unsigned LENGTH,                    // Shift register length
>
class sct_buffer {};
```

In the process reset section shift register ```reset()``` method should be called.
Shift register has ```T put(const T& new_val)``` method which does: shift-in new value, pop-out and return oldest value.
There are two ```peek``` methods to get oldest value of the shift register and get shift register element by index.

```cpp
/// Shift-in new value, pop-out and return oldest value
T put(const T& new_val); 
/// Peek shift register output (oldest value)
const T peek() const;
/// Peek shift register element by index (const)
const T peek(Idx_t i) const;
```


# Pipeline register

Pipeline register (```sct_pipe```) is intended to pipeline combinational logic and enable re-timing feature of a logic synthesis tool.
Pipeline register can be used in one method or thread process as well as put in one process and get in other process. Pipeline register is normally added to sensitivity list of process where put or get done. 

Pipeline register supports put bubbles and get backpressure. If there is no get, but some empty registers, they are shifted to provide next request put.

The pipeline register implements ```sct_fifo_if``` interface. It has size template parameter which is a positive number.
```cpp
template<
    class T, 
    unsigned N,                         // Number of pipeline registers, one or more
    class TRAITS = SCT_CMN_TRAITS,      // Clock edge and reset level traits
    bool TLM_MODE = SCT_CMN_TLM_MODE>   // RTL (0) or TLM (1) mode
>
class sct_pipe {};
```

The pipeline register can have input or output registers, which are not used for re-timing.  
```cpp
sct_pipe(const sc_module_name& name, 
         bool addInReg = 0,             // Add input register not moved by re-timing 
         bool addOutReg = 0,            // Add output register not moved by re-timing
         const std::string& rtlName = "DW_pl_reg") // Pipeline register instantiated component name
``` 

Typical use case for pipeline register is combinational logic re-timing in method process.
```cpp
void methProc() {
    run.reset_get();
    resp.reset_put();
    pipe.reset();

    if (pipe.ready() && run.request()) {
        T data = compute(run.get());     // Heavy computation to be pipelined
        pipe.put(data);
    }
  
    if (pipe.request() && resp.ready()) {
        resp.put(pipe.get());
    }
}
```

# Register

Register is used to add state for SCT_METHOD process. Register is written in one method process and could be read in the same or other method process(es). Register is normally added to sensitivity list of process where it is read. Register can be read in thread process.

Register has the same template parameters as Target/Initiator:
```cpp
template<
    class T,                            // Payload data type 
    class TRAITS = SCT_CMN_TRAITS,      // Clock edge and reset level traits
    bool TLM_MODE = SCT_CMN_TLM_MODE>   // RTL (0) or TLM (1) mode
class sct_register {};
```

Register has the following methods:
```cpp
// Reset register, set it value to stored at last clock edge
void reset();
// Write new value to register
void write(const T& data);
// Read value stored at last clock edge
T read();
// To skip using read()
operator T ();
```

Register can initiate a new request. That means an output request can depend on register state.

```cpp
sct_target<T>       targ{"targ"};
sct_register<T>     cntr{“cntr”};
explicit A(const sc_module_name& name) : sc_module(name) {
    targ.clk_nrst(clk, nrst);
    cntr.clk_nrst(clk, nrst);
    SC_METHOD(checkProc); sensitive << targ << cntr;
}

void checkProc() {
    cntr.reset();
    // Register accumulates received data up to N
    if (cntr.read() > N) {
        cntr.write(0); 
    } else 
    if (targ.get(data)) {
        cntr.write(cntr.read()+data); 
    }
}
```

# Clock, clock gate and clock gate signal

```sct_clock<>``` is implementation of clock source (generator) with enable/disable control. 
```cpp
    /// Enable clock activity, clock is enabled after construction 
    void enable();   
    /// Disable clock activity, can be called at elaboration phase to disable
    /// clock at simulation phase start
    void disable();    
    /// Register clock gate signals/ports to control clock activity.
    /// If any of the signals/ports is high, then clock is enabled
    void register_cg_enable(sc_signal_inout_if<bool>& enable);
    /// Get clock period    
    const sc_time& period() const;
```

In RTL mode ```sct_clock<>``` inherits ```sc_clock<>``` and has the same behavior (same event timing). That means clock events are 1 delta cycle delayed after clock period time.
TLM implementation of ```sct_clock<>``` provides notification aligned with other channels events. That means clock events happen at clock period time (no 1 delta cycle delay). Any thread process created with ```SCT_THREAD```, ```SC_THREAD```, and ```SC_THREAD``` macros can be driven by ```sct_clock<>```. These processes will be activated to execution (resumed) at clock period time. Therefore using ```sct_clock<>``` allows to mix approximate time and cycle accurate modules. 

Timed wait like ```wait(1, SC_NS)``` can be used in TLM mode in any thread process created with ```SCT_THREAD```, ```SC_THREAD```, and ```SC_THREAD``` macros (these processes has to be driven by ```sct_clock<>```). Timed wait cannot be used in RTL mode for any thread process.

Clock gate cell ```sct_clock_gate_cell``` and clock signal ```sct_clk_signal``` should be used together to connect clock input to gated clock source. ```sct_clk_signal``` is special signal without DC delay in written value becomes readable. 

The code example illustrates using ```sct_clock_gate_cell``` and ```sct_clk_signal```.
```cpp
SC_MODULE(A) {
    sc_in_clk               SC_NAMED(clk);
    sc_in<bool>             SC_NAMED(nrst); 
    sc_in<bool>             SC_NAMED(clk_enbl);
    sct_clk_signal          SC_NAMED(clk_out);
    sct_clk_gate_cell       SC_NAMED(clk_gate);
    sc_in<bool>             SC_NAMED(clk_in);

    explicit A(const sc_module_name& name) : sc_module(name) {
        clk_gate.clk_in(clk);        // Clock input
        clk_gate.enable(clk_enbl);   // Gate clock input 
        clk_gate.clk_out(clk_out);   // Gated clock output    
        clk_in(clk_out);
        
        SCT_THREAD(thrdProc, clk_in, nrst);   // Use clock input bound to gated clock
        async_reset_signal_is(nrst, 0);
}};
``` 

Clock gate cells can be sequentially connected to each other, gated clock output of one cell bound to clock input of anther cell.

In TLM mode, if all thread processes are created with ```SC_THREAD```/```SCT_THREAD``` macros, clock source(s) can be disabled. Disabling ```sct_clock``` allows to speed up simulation.
```cpp
sct_clock<>     clk{"clk", 1, SC_NS};
explicit A(const sc_module_name& name) : sc_module(name) {
    if (SCT_CMN_TLM_MODE) {
         clk.disable();
    }
}
```


# Flip-Flop Synchronizer

Synchronizer represent 1 or 2 Flip-Flops synchronizer with 1bit data (type ```bool```). Synchronizer can be used in thread and method processes. It implements ```sct_inout_if``` and has ```reset()``` function to be called in thread process reset section only.

```cpp
template <unsigned SyncType,    // Number of FF: 1 or 2
          bool RstVal,          // Reset value
          class TRAITS>
class sct_ff_synchronizer {};
```

Synchronizer should be bound to clock and reset, and added into sensitivity list of the process which does read.

```cpp
sct_target<T>           targ{"targ"};
sct_ff_synchronizer<2>  sync{"sync"};
sct_out<bool>           out{"out"};

explicit A(const sc_module_name& name) : sc_module(name) {
   sync.clk_nrst(clk, nrst);
   SC_METHOD(writeProc); sensitive << targ;
   SC_METHOD(readProc); sensitive << sync;
}
void writeProc() {
    sync = 0;
    if (targ.request()) {
        sync = targ.get();
    }
}
void readProc() {
    out = sync.read();
}
```

# Dual-clock FIFO

Dual clock FIFO (or CDC FIFO) is intended to provide Clock Domain Crossing communication. The CDC FIFO is normally instantiated between two modules in different clock domains. One module should have an Initiator, another module should have a Target, both bound to the CDC FIFO instantiated in their common parent module (see picture below).

The CDC FIFO SystemC implementation is used for cycle accurate and fast simulation. 

The CDC FIFO has the following template parameters:
```cpp
template <
    unsigned DATA_WIDTH,    // Width of the @push_data and @pop_data buses, 1..2048
    unsigned LENGTH,        // Number of words that can be stored in the FIFO, 4..1024
    class TRAITS,           // Clock edge and reset level
    unsigned PUSH_SYNC,     // Push flag synchronization mode (1/2/3 registers)
    unsigned POP_SYNC       // Pop flag synchronization mode (1/2/3 registers)
>
class sct_cdc_fifo {};
```

There is a code example with CDC FIFO which bound to target/initiator.

```cpp
    sc_in<bool>         clk1{"clk1"};
    sc_in<bool>         clk2{"clk2"};
    sc_in<bool>         nrst{"nrst"};

    Domain1  d1{"d1"};
    Domain1  d2{"d2"};
    sct_cdc_fifo<DATA_WIDTH, FIFO_LEN>  fifo{"fifo"};

    explicit Dut(const sc_module_name& name) : sc_module(name) {
        d1.clk(clk1);
        d2.clk(clk2);

        fifo.clk1(clk1); 
        fifo.clk2(clk2); 
        fifo.nrst(nrst);

        fifo.bind(d1.init);
        fifo.bind(d2.targ);
    }
```

The CDC FIFO has interface which are compatible with target/initiator interfaces and consist of request, ready and data signals. 


# External ports

External ports used to connect an IP to other IPS and SoC fabric. Examples of external ports are: AMBA AXI, AHB, and APB ports, OCP, IOSF. There are master and subordinate ports designated with ```_m_``` and ```_s_``` letter correspondently. 

Any external port inherits the base port.
```cpp
/// Subordinate base port
template <class TRAITS,                      // Base port traits
          class ADDR_TRANSL,                 // Address translation module 
          bool TLM_MODE_ = SCT_CMN_TLM_MODE>  
class base_s_port {};

/// AXI-lite subordinate port
template <class TRAITS,                     // AXI-lite specific traits
          class ADDR_TRANSL>
class axi4_lite_s_port<TRAITS, ADDR_TRANSL, ...> :
    public base_s_port<TRAITS, ADDR_TRANSL, ...> {};
```

Each specific port has its own set of traits.
```cpp
// AXI-lite specific traits
template <
    unsigned PORT_TYPE_,           // AXI port type (see EPortType::PT_AXI*)
    unsigned DATA_WIDTH_,          // data width, bits
    unsigned PORT_ADDR_WIDTH_,     // AXI address width, bits
    unsigned PORT_TAG_WIDTH_,      // AXI ID width, bits
    unsigned CORE_ADDR_WIDTH_ = 0, // core word-line address width, 0 - auto-calculate
    unsigned ATOMIC_MODE_     = 0, // atomic mode, see EAtomicMode
    bool     USE_BYTEEN_      = 1, // use byte enables (optional for AXI4-Lite)
    bool     USE_ERR_RESP_    = 0, // use error responses (rresp/bresp)
    bool     USE_ERR_INT_     = 1, // use error interrupt outputs
    bool     USE_CLK_GATING_  = 1, // use clock gating support
    unsigned REQ_RSP_CNTR_W_  = 6  // request/response counter width
>
struct axi_s_port_traits {};
```

Base port and its inheritors have inputs/outputs pins (in RTL mode) for external connect and Target/Initiator for request and response to be processes in the IP design. Request and response format is given below. 
```cpp
template <...>
class base_s_port {
    // Request: (tag, byte enable, write data, address, operation 1 - write / 0 - read)
    typedef CoreReq <TR::CORE_ADDR_WIDTH, TR::DATA_WIDTH, TR::CORE_BE_WIDTH, TR::CORE_TAG_WIDTH> CoreReq_t;
    using ReqBits_t = typename CoreReq_t::Bits_t;    // sc_uint or sc_biguint with request width

    // Response: (tag, read data, error, operation 1 - write / 0 - read)
    typedef CoreRsp <TR::DATA_WIDTH, TR::CORE_TAG_WIDTH> CoreRsp_t;
    using RspBits_t = typename CoreRsp_t::Bits_t;    // sc_uint or sc_biguint with response width

    sct_initiator<ReqBits_t>    core_init{"core_init"};
    sct_target<RspBits_t>       core_targ{"core_targ"};
    ...
}
```

An external port is usually instantiated at top module. There is an example of AXI-lite subordinate port instantiation.
```cpp
class Top : sc_module {
   using AXIL_TRAITS = axi_s_port_traits<PT_AXI4LT, 32, 20, 4, 16>;
   using AxilPort_t = axi4_lite_s_port<AXIL_TRAITS>;
   AxilPort_t port{"port"};

   explicit Top(const sc_module_name& name) : sc_module(name) {
        port.clk_nrst(clk, nrst);
        // Bind port Target/Initiator to this Top module or its child module
        port.core_init.bind(...);  
        port.core_targ.bind(...);
   }
};
```


# Reset

## Reset section

In thread process reset logic initializes registers, local variables and output signals. This logic should be placed in reset section (code scope before first ```wait()```). 
```cpp
sct_out<T> o{"o"};
sct_signal<T> s{"s"};
void thrdProc() {
    // Reset section
    int a = 0;   // Local variable
    s = 0;       // Register 
    o = 0;       // Output 
    wait();
    while (true) {
        ...
        wait(); 
    } 
}
```

In method process initialization logic initializes local variables and output signals. This logic is normally be placed in the beginning of the process. 
```cpp
sct_out<T> o{"o"};
void methdProc() {
    // Initialization section
    int a = 0;   // Local variable
    o = 0;       // Output 
    ...
    a = i + 1;
    if (s) o = a;
}
```

Initialization logic in method process could be merged with its behavior logic based on inputs and registers. Such code style can have better simulation performance.
```cpp
sct_in<T> i{"i"};
sct_out<T> o{"o"};
void methdProc() {
    int a = i+1;      // Local variable
    o = a ? s : 0;    // Output 
    ...
}
```

The communication channels also need to be reset with specified ```reset()```, ```reset_get()``` and ```reset_put()``` methods. In thread process every channel used in this process should be initialized in the reset section.
```cpp
sct_initiator<T>  init{"init"};
sct_target<T>     targ{"targ"};
sct_fifo<T, 2>    fifo{"fifo"};
void thrdProc() {
    init.reset();
    targ.reset();
    fifo.reset_put();  // If FIFO used for put
    fifo.reset_get();  // If FIFO used for get
    fifo.reset();      // If FIFO used for get and put both
    wait();
    while (true) {
        ...
        wait(); 
    } 
}
```

In method process every channel used in this process is initialized in the beginning of the process or assigned at all execution path in the process code. Having no explicit reset for registers, signals, output ports and synchronizers can improve simulation performance.
```cpp
sct_initiator<T>  init{"init"};
sct_target<T>     targ{"targ"};
sct_register<T>   reg1{"reg1"};
sct_register<T>   reg2{"reg2"};
void methProc() {
    init.reset();
    reg1.reset();        
    T val = targ.get();  // targ is accessed at all path, no reset required
    if (val > 0) {
        reg1 = val;      // reg1 accessed at some paths only, reset required
        init.put(val);   // init accessed at some paths only, reset required
    }
    reg2 = val + 1;      // reg2 is accessed at all path, no reset required 
}
```

## Reset control 

Reset signal can be asserted/de-asserted in TB and DUT processes as well. To have the same simulation time in RTL and TLM modes it needs to follow the rules given in this section.

If reset control thread is in TB, it could control reset based on time period and be non-sensitive to any channels. In this case such a thread should be ```SC_CTHREAD``` in RTL mode and ```SC_THREAD``` in TLM mode. To avoid extra activation in TLM mode, this thread should wait for a specified time instead of clock events.

```cpp
SC_MODULE(A) {
   SC_CTOR(A) {
       // Thread not sensitive to anything
       #ifdef SCT_TLM_MODE
          SC_THREAD(resetProc);
       #else
          SC_CTHREAD(resetProc, clk_in.pos());
       #endif
   }
   #define rstWait(N) if (SCT_CMN_TLM_MODE) wait(N, SC_NS); else wait(N);
   void resetProc() {
        nrst = 0; 
        rstWait(3);
        cout << sc_time_stamp() << " " << sc_delta_count() << " de-assert reset\n";
        nrst = 1; 
        rstWait(5);
        ...
   } 
};
```

If reset control thread is sensitive to any channels, it should be ```SCT_THREAD``` and have ```dont_initialize()``` in RTL mode. Such a thread can also be a normal test thread which provides stimulus and checks results:
```cpp
SC_MODULE(A) {
   SC_CTOR(A) {
       // Thread sensitive to SS channels
        SCT_THREAD(resetProc, clk);
        #ifndef SCT_TLM_MODE
            dont_initialize();
        #endif
        sensitive << s;
   }
   sct_signal<unsigned>  s{"s"};
   void resetProc() {
        nrst = 0; 
        while (s.read() < 3) {s = s.read()+1; wait();}
        cout << sc_time_stamp() << " " << sc_delta_count() << " de-assert reset\n";
        nrst = 1; 
   }
```


## Specify clock edge and reset level

Clock edge and reset level normally are the same for the design. To update them for whole design ```SCT_CMN_TRAITS``` should be defined. For example, to set negative edge and positive reset level:
```cpp
#define SCT_CMN_TRAITS SCT_NEGEDGE_POSRESET   // Set negative edge and positive reset level
```

To specify clock edge and reset level for individual library modules, template parameters should be used, for example:
```cpp
sct_target<T, SCT_NEGEDGE_POSRESET>       run{"run"};
sct_initiator<T, SCT_POSEDGE_NEGRESET>    resp{"resp"};
```



# Design architecture

## Array/vector of SingleSource channels

Array of SingleSource channels can be implemented with ```sc_vector```. First parameter of ```sc_vector``` is name, second parameter is number of elements (should be a compile time constant). To provide additional parameters to single source channels, it needs to use lambda function as third parameter of ```sc_vector```. 
```cpp
static const unsigned N = 16;
using T = sc_uint<16>;
sc_vector<sct_target<T>>       targ{"targ", N};        // Two parameters 
sc_vector<sct_initiator<T>>    init{"init", N,         // Three parameters
      [](const char* name, size_t i) {                 // Lambda function         
           return sc_new<sct_initiator<T>>(name, 1);   // Initiator with sync register
      }}; 
```

## Target and initiator in top module

Target and Initiator can be instantiated in top module to be externally connected to the correspondent modules in testbench. Such top module is synthesizable with input/output ports for the Target/Initiator instances.

Top module can contain Target which is not always ready and has no synchronous register. Top module can contain initiator which has no synchronous register. Top module cannot contain MultiTarget or MultiInitiator. Vector (```sc_vector```) of Target/Initiator in top module is supported.
For synchronous register in a top module Target/Initiator externally connected to a testbench, ICSC reports the corresponding error.

To connect a testbench Target/Initiator to the correspondent top module Initiator/Target normal ```bind``` function is used always except multi-language simulation. For multi-language simulation if DUT is in SystemVerilog and testbench is in SystemC language, the simulation tool generates a special SystemC wrapper for DUT top module. To connect this wrapper to SystemC testbench ```SCT_BIND_CHANNEL``` macro should be used. ```SCT_BIND_CHANNEL``` macro cannot be applied to Target/Initiator with record type. 
For synchronous register in a testbench Target/Initiator connected to the DUT ```SCT_BIND_CHANNEL``` macro reports the corresponding error at elaboration phase of simulation.

```cpp
// Include DUT module generated wrapper or SystemC header 
#ifdef RTL_SIM
    #include "DUT.h"          // Multi-language simulation, include generated wrapper
#else 
    #include "MyDut.h"        // SystemC simulation and synthesis, include designed header
#endif

template<class T>
class MyModule : public sc_module {
   DUT                       dut{"dut"}; 
   sct_target<T>             targ{"targ"};
   SC_CTOR(MyModule) {
       // Bind targ to init in dut module 
       #ifdef RTL_SIM
           SCT_BIND_CHANNEL(dut, init, targ);         // Multi-language simulation
       #else 
           targ.bind(dut.init);                       // SystemC simulation and synthesis
       #endif
   }
}
```

### Array of Target/Initiator in top module

Array of Targets/Initiators supported in any module including top module. Instead of C++ array ```sc_vector``` should be used (C++ array is not supported). 
To bind the Targets/Initiators ```SCT_BIND_CHANNEL``` macro with 4 parameters is provided.

```cpp
// Include DUT module generated wrapper or SystemC header 
#ifdef RTL_SIM
    #include "DUT.h"         // Multi-language simulation, include generated wrapper
#else 
    #include "MyDut.h"       // SystemC simulation and synthesis, include designed header
#endif

template<class T, unsigned N>
class MyModule : public sc_module {
   DUT              dut{"dut"}; 
   sc_vector<sct_target<T>>    targ{"targ", N};
   SC_CTOR(MyModule) {
       // Bind all elements of targ to elements of init in dut module 
       #ifdef RTL_SIM
           SCT_BIND_CHANNEL(dut, init, targ, N);   // Multi-language simulation
       #else 
           for (unsigned i = 0; i != N; ++i)  
               targ[i].bind(dut.init[i]);          // SystemC simulation and synthesis
       #endif
   }
}
```

## Hierarchical connection of Target and Initiator

Target and Initiator can be connected through module hierarchy from child module up to parent module. That is possible explicitly or with ```sc_port``` of  Initiator/Target. 

There is an example of explicit binding Target to Initiator through module hierarchy:

```cpp
template<class T>
struct Child : public sc_module {
    sct_target<T>       run{"run"};
};

template<class T>
struct Parent: public sc_module  {
    Child<T>           child{"child"};
    SC_CTOR(Parent) {}
};

SC_MODULE(Top)  {
    sct_initiator<T>    resp{"resp"};
    Parent<T>           parent{"parent"};
    SC_CTOR(Top) {
        parent.child.run.bind(resp);
}};
```

Ports (```sc_port```) of Target/Initiator contain pointer to them. To bind Initiator to Target through ports it needs to use ```get_instance()``` method which provides Target/Initiator from its port, see option (1) example below. Method ```bind()``` through ```sc_port``` can be called directly, see option (2) in the example below. 

```cpp
template<class T>
struct Child : public sc_module {
    sct_target<T>       run{"run"};
};

template<class T>
struct Parent: public sc_module  {
    sc_port<sct_target<T>>       run;   
    Child<T>                     child{"child"}; 
    SC_CTOR(Parent) {
        run(child.run);   // Bind port to child module initiator
}};

SC_MODULE(Top)  {
    sct_initiator<T>    resp{"resp"};
    Parent<T>           parent{"parent"};
    SC_CTOR(Top) {
        resp.bind(parent.run->get_instance());   // (1) Use get_instance() to get Initiator from its sc_port  
        parent.run->bind(resp);                  // (2) Call bind() of Initiator through its sc_port       
}};
```

Process which calls Target/Initiator functions should be in the module where Target/Initiator declared. If a process calls Target/Initiator through its port (```sc_port<sct_target>```/```sc_port<sct_initiator>```) the process module and target initiator module should be synthesized in the same parent module.


## Module interconnect with FIFO

Connection between modular interfaces inside of common parent module can be done with FIFO. One modular interface should have a FIFO instance and other modular interface should have a FIFO port (```sc_port< sct_fifo<> >```).
The same can be done if the FIFO is instantiated in the parent module.

```cpp
template<class T, unsigned N>
struct A : public sc_module, sc_interface {
   sct_fifo<T, N>    SC_NAMED(fifo);
   ...
};

template<class T, unsigned N>
struct B : public sc_module, sc_interface {
   sc_port<sct_fifo<T, N>>    SC_NAMED(fifo_port);
   ...
};

struct Parent : public sc_module{
   A<int, 3>   SC_NAMED(a);
   B<int, 3>   SC_NAMED(b);
   SC_CTOR(Parent) {
      b.fifo_port(a.fifo);    // Bind FIFO port to FIFO instance
   }
};
```


## Record as data type in SingleSource channels

Record is supported as data type in all SingleSource channels. The record should comply SystemC requirements for records used in signal/port: the record should have default constructor w/o parameters, ```operator==()```, ```operator<<(std::ostream)``` and ```sc_trace()``` implemented.

```cpp
struct Rec_t {
    bool enable;
    sc_uint<16> addr;    
    Rec_t() : enable(false), addr(0) {}   // Default constructor
    bool operator == (const Rec_t& other) const {
        return (enable == other.enable && addr == other.addr);
    }
};
namespace std {
inline ::std::ostream& operator << (::std::ostream& os, const Rec_t& r) {
    os << r.enable << r.addr; return os;}
}
namespace sc_core {
void sc_trace(sc_trace_file* , const Rec_t& , const std::string&) {}
}
...
// Target with record payload
sct_target<Rec_t>   	SC_NAMED(targ);
sct_initiator<Rec_t>    SC_NAMED(init);

void methProc() {
   targ.reset_get();
   if (targ.request()) {
      Rec_t data = targ.get();
      doSomething(data);
   }
}

void thrdProc() {
   init.reset_put();
   wait();
      Rec_t data = getSomeData();
      init.b_put(data);
      wait();
   }
}
```

# TLM mode

## Difference between RTL and TLM simulations

RTL simulation provides precise result and is equivalent to simulation of the generated SV. TLM simulation is approximate time, i.e. channels in TLM mode have implementation optimized for speed. 
TLM FIFO is equivalent to RTL FIFO, so there is no difference. TLM Target/Initiator pair is equivalent to RTL only for default parameters (no always ready, no sync  registers, no FIFO added). If always ready used, sync registers, or FIFO added, TLM Target/Initiator pair differs from RTL one. 
TLM Pipe is implemented as FIFO (```sct_prim_fifo```) that significantly differs from RTL implementation.

TLM mode can be used whenever exact time of simulation events is not important. TLM mode can be used for functional modelling such as hardware or software debugging. For performance modelling TLM mode can be used if 100% precise result is not required, otherwise RTL mode should be used.


## TLM mode process sensitivity

If a design is used in TLM mode, it needs to ensure process sensitivity lists follow special rules. In TLM mode any process is activated by event notification from SingleSource channels and SystemC signals/ports. To catch an event notification, the correspondent process should be waiting for this event. That requires both of the following:
1. All the events are added into process sensitivity list;
1. Activated process goes to a next state where an event notification happens in some future. 

To satisfy first condition is it enough to add all channels accessed in the process into process sensitivity list. The second condition is more tricky and requires analysis of all inter-process communications. Lets discuss the second condition in more details.

Process activation events can be notified by the process itself and by other processes (for sequential process only). Other process notifies the events independently or in response to our process actions. The means any process activation should notify enough events (no event could be  enough) to get notification of its activation events back.

Initiator, Target and FIFO notifies the sequential or combinational process which performs put or get one more time. In the next example ```threadProc``` executes infinitely by self-notification. That allows the process puts to non-full FIFO multiple times w/o get from this FIFO. 

Sequential process example.
```cpp
// sct_single_fifo/method_test3.h
void threadProc() {
    fifo.reset_put();
    wait();
    while (true) {
       fifo.b_put(val);    // Activate this process again and again
       wait();
    }
}
```
Combinational process example.
```cpp
// sct_single_fifo/method_test3.h
void methPutProc() {
    mfifo.reset_put();
    if (mfifo.ready()) mfifo.put(val);
}

void methGetProc() {
    mfifo.reset_get();
    // Do nothing to let @methPutProc fill the FIFO
}
```


In the following example there are two processes which activates one to each other through the FIFO. In ```producerProc``` the FIFO is accessed to notify the event for ```consumerProc``` in state 0 and 1. ```consumerProc``` is activated by the FIFO event and access (get) the FIFO which notifies ```producerProc``` back. In state 2 where no event is notified, so ```consumerProc``` is not activated and ```producerProc``` is not activated more -- simulation hangs up. 

```cpp
struct Top : public sc_module {
    sct_fifo<T, 2>      fifo{"fifo", 1};     // Pipelining register for request
    explicit Top(const sc_module_name& name) : sc_module(name) {
        fifo.clk_nrst(clk, nrst);
        SC_THREAD(producerProc); sensitive << fifo.PUT;   // Process puts to FIFO
        async_reset_signal_is(nrst, 0);
        SC_METHOD(consumerProc); sensitive << fifo.GET;   // Process gets from FIFO   
    } 
}

void producerProc() {
    fifo.reset_put();
    wait();      // STATE 0
    while (true) {
       if (fifo.ready()) {           
          fifo.put(getSomeVal());
       }
       wait();   // STATE 1
       // Do nothing, no event notified this cycle
       wait();   // STATE 2
    }
}
void consumerProc() {
    fifo.reset_get();
    if (fifo.request()) {
       doSomething(fifo.get());
    }
}
```

The next example demonstrates sequential process which activates itself with FIFO channel. Any put and get to/from the FIFO notifies the event to activate the process. In this process any value except ```42``` is put to the FIFO. If value is ```42```, nothing is put and no event is notified, so the process is not activated and simulation hangs up.

```cpp
struct Top : public sc_module {
    sct_fifo<T, 5>      fifo{"fifo"};
    explicit Top(const sc_module_name& name) : sc_module(name) {
        fifo.clk_nrst(clk, nrst);
        SC_THREAD(storeProc); sensitive << fifo;  // Process puts and gets to FIFO
        async_reset_signal_is(nrst, 0);
    }
}

void storeProc() {
    fifo.reset();
    wait();
    while (true) {
       T val = getSomeValue();
       if (fifo.ready() && val != 42) {   
          fifo.put();
       } else {
          // Do nothing if @val is 42, no event notified this cycle
       }  
       wait();
       if (fifo.request()) {
          doSomething(fifo.get());
       }
    }
}

``` 

The next example shows how to create a counter process with signal channel only. Using ```sct_signal``` instead of ```sc_signal``` is required to avoid process is sensitive to it in RTL mode.

```cpp
struct Top : public sc_module {
    sct_signal<T>      cnt{"cnt"};
    explicit Top(const sc_module_name& name) : sc_module(name) {
        SCT_THREAD(cntProc, clk.pos());     // Second parameter required for RTL mode to know clock edge 
        sensitive << cnt;                   // Process read and write signal
        async_reset_signal_is(nrst, 0);
    }
}

void cntProc() {
    cnt = 0;
    wait();
    while (true) {
       // Updating @cnt notifies the process itself 
       cnt = (cnt.read() != 255) ? cnt.read() + 1 : 0;
       wait();
    }
}

``` 

A typical mistake in TLM mode is waiting for negotiation of a channels ready or request. In the example below, if the FIFO is ready no event is notified. As soon as FIFO could be ready more than one cycle, that could leads to simulation hangs up.

```cpp
void storeProc() {
    fifo.reset();
    wait();
    while (true) {      
       ...
       if (!fifo.ready()) { 
          // Do something that activates other processes
       } else {
          // Do nothing, no event notified this cycle
       }  
       wait();
    }
}

``` 

Joining multiple request and ready channel status in one condition normally is correct.
```cpp
   ...
   if (fifo.ready() && init.request()) { 
       // Do something that activates other processes
   } else {
       // Do nothing, no event notified this cycle
   }  
   ...
```

As soon as timing diagram in TLM mode can differ from RTL mode, it is prohibited to process logic assumes the specific number of cycles. 
Instead of that, process logic and process interconnect should based on operations with channels. The following example has a magic wait which can lead to different result in RTL and TLM modes.

```cpp
   ...
   if (fifo.ready()) {
      wait(3);       // Can lead to incorrect results
      fifo.get();
   }
   ...
```



# Debug

Development and most of the debug is intended to be done in RTL mode. After RTL mode tests passed, TLM mode could be used for faster simulation. If TLM mode behavior differs from RTL mode, it could be debugged with C++ debugger or in commercial simulators.

There is a problem with multiple put/get to a channel in the same cycle. That leads to only last data is stored. To detect this error ```SCT_TLM_DEBUG``` compile time define option is provided. This option can be used in TLM mode:
```make
target_compile_definitions(design-tlm PUBLIC SCT_TLM_MODE SCT_TLM_DEBUG)
```


To debug TLM mode in the simulation tool, ```EXTR_SIM_DEBUG``` option should be defined for ```syscan``` tool:
```make
syscan ... -cflags "-DEXTR_SIM_DEBUG" ... 
```

```EXTR_SIM_DEBUG``` option enables debug signals in ```sct_prim_fifo``` which implements target, initiator and FIFO in TLM mode. There are ```core_req```, ```core_ready``` and ```core_data``` signals, similar to signals between target/initiator.
```sct_prim_register``` which implements register in TLM mode has ```curr_val``` which is value to be read.    ```sct_prim_synchronizer``` which implements synchronizer in TLM mode has ```curr_val``` and ```next_val```. ```curr_val``` is value to be read, ```next_val``` is just written value.

TLM mode simulation in the simulation tool can be used as normal SystemC simulation.

SystemC provides functions to create VCD trace file with specified channels. To enable VCD traces it needs to define ```DEBUG_SYSTEMC```. There are ```trace()``` implementations for FIFO, Initiator and Target channels. To specify required channels it needs to call ```trace()``` for them in the module constructor.

```cpp
SC_MODULE(MyModule) {
    sc_trace_file*      tf;
    sct_fifo<T,2>       fifo{"fifo"};
    sct_target<T>       targ{"targ"};
    
    SC_CTOR(MyModule) {
        tf = sc_create_vcd_trace_file("trace1");   // Create and opene trace file
        fifo.trace(tf);                            // Add fifo to trace
        targ.trace(tf);                            // Add targ to trace
    }
    ~MyModule() {
        sc_close_vcd_trace_file(tf);               // Close trace file
    }
}
```