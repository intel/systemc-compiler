# SystemC coding rules

## Project structure

Project consists of main source file -- `sc_main.cpp` and multiple header files.
The main source file has `sc_main` function wihch is entry point like `main` funciton in C++ projects.
In `sc_main` there is testbench top module instantion. SystemC design normlly has one top module. The design top module can be instantiated inside of `sc_main` or inside of one of testbench modules. Clocks are normally instantiated in `sc_main`, but also could instantiated inside of testbench.


## Module hierarchy

SystemC design consists of modules and modular interfaces instances
which are organized into a module hierarchy. *Module* is a C++ class or
structure which inherits ```sc_module``` class. *Modular interface* is a C++
class or structure which inherits `sc_interface` and ```sc_module```
classes. If a class inherits `sc_interface`, but does not inherit
```sc_module``` we call it *pure interface*. Pure interface is not a part of
module hierarchy. Each interface and module, except top module, is
instantiated inside of *parent module*. Modules and modular interfaces
can be instantiated at stack or be dynamically allocated in heap with
`operator new`.

The difference between module and modular interface from ICSC viewpoint
is that modular interface is flatten into parent module where it is
instantiated. That means there is no modular interface instance in
generated SystemVerilog, but its fields and processes are instantiated
into the parent module. In other words, modular interface is a kind of
module which needs to be flatten in its parent module.

The SystemC design may have one or several top module instances in
`sc_main` function or another module, but only one of them will be
translated into SystemVerilog. The top module to take by ICSC is
specified in `ELAB_TOP` option of `svc_target`. If top module
instantiated in `sc_main` function and there is no more modules
instantiated, `ELAB_TOP` option could be omitted.

Top module contains child module(s). Every module may inherit another
module(s), interface(s) or class(es) according with C++ rules. Multiple
inheritance and virtual inheritance is supported by ICSC tool.

Module, interface, class or structure can be template type. Template
types, template specialization and instantiation specified by C++ rules.
All that is supported by ICSC tool.

In accordance with SystemC LRM, modules/modular interfaces and pointers
to them cannot be function parameters, cannot be returned from function,
and cannot be used as signal/port template type.

## Module interconnect

For module interconnect SingleSource channels like ```sct_target``` and ```sct_initiator``` as well as signals ```sct_signal```, ports ```sct_in``` and ```sct_out``` can be used. Having explicit pointers or references to another module fields or methods considered as bad programming style and
should be avoided.

Child module input/output ports could be directly connected to
corresponded input/output ports of its parent module. Child ports, not
connected to any signal/port, are promoted to its parent module and
further up to top module. That practically means, unconnected port
becomes the same type port of top module in generated SystemVerilog.

```cpp
// SystemC child module port promotion to top module
SC_MODULE(Child) {
    sc_in_clk   clk{"clk"};
    sc_in<sc_int<16>>  in{"in"};       // Not connected
    sc_out<sc_int<16>> out{"out"};     // Not connected
    
    SC_CTOR(Child) {}
};

SC_MODULE(Top) {
    sc_in_clk   clk{"clk"};
    Child child_inst{"child_inst"};
    
    SC_CTOR(Top) {
        child_inst.clk(clk);
    }
};
```

Two modules with the same parent module can be connected through:

1.  triple of ```sct_in<T>```, ```sc_signal<T>```, ```sc_out<T>```
2.  pair of ```sct_in<T>```, ```sc_signal<T>```
3.  pair of ```sc_out<T>```, ```sc_signal<T>```

In case (1) ```sc_in<T>``` port is in module A, ```sc_out<T>``` port in module B
and ```sc_signal<T>``` in their parent module. These input and output ports
bound to the signal in the parent module constructor. In case (2)
```sc_in<T>``` port is in module A, ```sc_signal<T>``` in module B. The input
port connected to the signal in the parent module constructor. In case
(3) ```sc_signal<T>``` is in module A, ```sc_out<T>``` port in module B. The
output port connected to the signal in the parent module constructor.

```cpp
// SystemC child modules connected to each other
SC_MODULE(Producer) {
    sct_signal<bool>         req_sig{"req_sig"};
    sct_out<bool>            resp{"resp"};
    sct_out<sct_int<16>>      data{"data"};
    
    SC_CTOR(Producer) {}
};

SC_MODULE(Consumer) {
    sct_in<bool>             req{"req"};
    sct_signal<bool>         resp_sig{"resp_sig"};
    sct_in<sct_int<16>>       data{"data"};
    
    SC_CTOR(Consumer) {}
};

SC_MODULE(Parent) {
    Producer prod{"prod"};
    Consumer cons{"cons"};
    
    sct_signal<sct_int<16>>   data{"data"};

    SC_CTOR(Parent) {
        cons.req(prod.req_sig);     // in-to-signal  (2)
        prod.resp(cons.resp_sig);   // out-to-signal (3)       
        prod.data(data);            // in-to-signal-to-out (1)
        cons.data(data);
    }
};
```

Port can be connected to a signal through module hierarchy. That means
the port and signal can belong to different modules, not necessary to
child and parent or two children of the same parent.

```cpp
SC_MODULE(A) {
    sct_in<T>     SC_NAMED(i);     
};
SC_MODULE(B) {
    A            SC_NAMED(mod_a);
};
SC_MODULE(C) {
    sct_signal<T> SC_NAMED(s);
    B            SC_NAMED(mod_b);
    SC_CTOR(C) {
       mod_b.mod_a.i(s);   // Cross-binding of port "i" to signal "s"
    } 
};
```

## Modular interface access

A module process can access its child modular interface instance fields
and call methods. That is possible as interface fields and methods are
moved to the parent module in generated SystemVerilog. Direct accessing
fields of another module considered as bad programming style and should
be avoided.

In general case a module can have pointer to another module or
```sc_port<IF>``` connected to another module. It can access field and call
methods of the pointee module if both of these modules are flatten in
the same module in SystemVerilog. That can be parent and its child
instance of modular interface or two child instances of modular
interface(s) in the same parent module.

```sc_port<IF>``` is special case of port which provides pure interface IF.
```sc_port<IF>``` can be connected to a modular interface, which implements
all abstract methods of ```IF```. With ```sc_port<IF>``` it is possible to call
methods of the IF that limits access to connected module. In that access
via ```sc_port<IF>``` differs from access via modular interface pointer.

Other ways of call of another module methods or access another module
fields are prohibited.

## Module constructor

Any module/modular interface must have at least one constructor.
Module/modular interface constructor must have ```sc_module_name```
parameter which can be passed to ```sc_module``` inheritor or just not used.
Module/modular interface constructor can contains arbitrary C++ code
inside.

Module/modular interface constructor body can contain:

-   processes creation with ```SCT_METHOD```, ```SCT_THREAD``` macros,
-   non-SystemC objects allocation in dynamic memory with ```sc_new``` operator,
-   child modules and other SystemC objects allocation with ```new``` operator,
-   child module port bindings to port/signals,
-   function calls of this module or its child modules.

It is not recommended to use any dynamic memory allocation or pointers.

Module member constants can be initialized in place, in module
constructor initialization list. 
Global and member constants are translated into ``localparam``` in SystemVerilog. Member constants of any integral type should be 64 bit or less (stored in uint64/int64 field). Such constants with more than 64 bit has unknown value after elaboration, therefore error is reported.
Local constants in process function are translated to SystemVerilog local variables. Such constants can be more than 64bit.

Non-constant module fields also can be initialized in constructor or in
place. If a module field has integral type and not modified in any
process, it considered as simulation time constant and translated into
```localparam``` in SystemVerilog.

``` cpp
// Constants initialized in and after constructor
class MyModule : public sc_module {
    const sct_uint<8> A = 0;          // In-place initialization
    const int B[4] = {0,1,2,3};      // In-place initialization
    const bool C;       
    const unsigned* D = nullptr;
    int E;
    
    MyModule(const sc_module_name& name, int par) :
        sc_module(name), 
        C(par == 42)                 // In initialization list
    {
        setParam(par);
    }
    
    unsigned d;
    void setParam(unsigned par) {
       d = par;
    }        
}
MyModule mod("mod", 12, true);
```

## Processes

SystemC design with single source library can use method and thread processes created with ```SC_METHOD``` and ```SC_THREAD``` correspondently. It is recommended to use ```SCT_METHOD``` and ```SCT_THREAD``` macros instead of them: 
 * ```SCT_METHOD(proc)``` -- combinational method process, same as ```SC_METHOD(proc)```,
 * ```SCT_METHOD(proc, clk)``` -- sequential method process with synchronous reset,
 * ```SCT_METHOD(proc, clk, rst)``` -- sequential method process with asynchronous reset,
 * ```SCT_THREAD(proc)``` -- sequential thread process, same as  ```SC_THREAD(proc)```,
 * ```SCT_THREAD(proc, clk)``` -- sequential thread process with explicit clock,
 * ```SCT_THREAD(proc, clk, rst)``` -- sequential thread process with explicit clock/reset.

```SCT_METHOD(proc)``` creates a combinational process operates with single source channels, signals/ports. 
```SCT_METHOD(proc, clk)``` and ```SCT_METHOD(proc, clk, rst)``` creates a sequential method process.
```SCT_THREAD(proc)``` creates a sequential process operates with single source channels, use this macro as default one if no signals and no ports are read i nthe process. 
```SCT_THREAD(proc, clk)``` creates a process operates with single source channels and reads single source signals (```sct_signal```) / ports (```sct_in```/```sct_out```).
```SCT_THREAD(proc, clk, rst)``` creates **a universal sequential process** which can access single source channels and/or reads single source signals (```sct_signal```) / ports (```sct_in```/```sct_out```).
```SC_CTHREAD``` macro is normally not used. 

Difference between sequential method and thread processes is in simulation speed, method process is faster. Thread process allows to use ```wait()``` to introduce multiple states, which can simplify the process function code. Sequential method process function should have reset section and sequential logic section, and cannot have ```wait()``` calls.
```cpp
void combMethod() {
   // combinational logic ...
}
void seqMethod() {
   if (rst) {
      // reset logic ...
   } else {
      // sequential logic ...
   }
}
void seqThread() {
   // reset logic
   wait();
   while (true)
      // sequential logic ...
      wait();         
      // optional sequential logic ...
      // multiple wait() calls allowed
   }
}
```

Clock and reset levels for a process are specified by ```SCT_CMN_TRAITS``` if clock/reset are explicitly provided. Otherwise clock and reset levels are taken from channels in the sensitivity list. Channels obtains clock and reset levels from ```SCT_CMN_TRAITS``` by default, that can be changed for individual channels.
All the channels used in a sequential process should use the same clock and edge as the process sensitive. A channel could have different reset or reset level than the process. 
If a thread process has reset signal(s), it should have the reset specification with ```async_reset_signal_is``` or/and ```sync_reset_signal_is```. 

Any process should be sensitive to all single source channels accessed and to all single source signals (```sct_signal```) / ports (```sct_in```/```sct_out```) read in its function code. Combinational method should be also sensitive to all SystemC signals (```sc_signal```) and ports (```sc_in```/```sc_out```) read in its function code. A process is never sensitive to reset.

```cpp
template <class T>
class MyModule : public sc_module {
   sc_in<bool>      clk{"clk"};
   sct_target<T>    targ{"targ"};
   sct_initiator<T> init{"init"};
   sct_signal<T>    s{"s"};

   explicit MyModule(const sc_module_name& name) : sc_module(name) {
        SCT_METHOD(combMethod);                      // Combinational method, same as SC_METHOD
        sensitive << init;                           // No reset in sensitivity

        SCT_METHOD(seqMethod, clk);                  // Sequential method with synchronous reset
        sensitive << init;                           

        SCT_METHOD(seqMethod, clk, nrst);            // Sequential method with asynchronous reset
        sensitive << init;                           

        SCT_THREAD(seqThread);                       // Sequential thread, sensitive to @sct_target only
        sensitive << targ;                           // Clock and reset taken from @sct_target
        async_reset_signal_is(nrst, 0);              // Reset specification required

        SCT_THREAD(seqThread, clk);                  // Sequential thread, sensitive to @sct_target and @sct_signal
        sensitive << targ << s;                      // Clock is explicitly provided for @sct_signal
        async_reset_signal_is(nrst, 0);              // Reset specification required

        SCT_THREAD(seqThread, clk, nrst);            // Most universal sequential thread, sensitive to @sct_signal here
        sensitive << s;                              // Clock and reset are explicitly provided
        async_reset_signal_is(nrst, 0);              // Reset specification still required
   }
};

```

If any process sensitive to a channel which is not read inside or not sensitive to a channel which is read inside, error reported by ICSC. The error is reported for single channels and for vector/array of channels, no individual channels in vector/array are considered here.

To add to process sensitivity prefer `operator <<` over `addTo()`, `addToPut()` and `addToGet()` methods, for example:
```cpp
    explicit MyModule(const sc_module_name& name) : sc_module(name) {
        SCT_METHOD(combMethod);                      
        sensitive << init;                   // Use sensitive << instead of addTo()
        init.addTo(sensitive);               // Avoid to use addTo()

        SCT_THREAD(seqThread); 
        sensitive << fifo.PUT;               // Use sensitive << with .PUT instead of addToPut()
        fifo.addToPut(sensitive);            // Avoid to use addToPut()
        sensitive << fifo.GET;               // Use sensitive << with .GET instead of addToGet()
        fifo.addToGet(sensitive);            // Avoid to use addToGet()
        async_reset_signal_is(nrst, 0); 
    }
```

### Method process

Method process created with `SCT_METHOD` or `SC_METHOD` marco in module constructor. 
Method process is combinational process without state. Method process should have sensitivity list with all signals and channels read in the process code.

``` cpp
// Method process example
void methodProc() {
    bool x;
    int i;
    i = a.read();
    x = i == b.read();
    sig = (x) ? i : 0;
}
```

### Method process with empty sensitivity 

Method process with empty sensitivity are typically used to assign
constant value signal/port initialization.

``` cpp
// SystemC method process with empty sensitivity
static const bool COND = true;
void emptySens()
{
    a = 0;
    if (COND) {
        b = 1;
    } else {
        c = 2;
    }
    int i = 1;
    d = (!COND) ? i : i + 1; 
}
```


### Method process with latch(es) 

Normally ICSC does not allow to have latch in SystemC source, but there
are some cases where latch is required. There is ICSC assertion
`sct_assert_latch` which intended to specify latch in method process. 
It suppresses ICSC error message for latch variable, signal or port.

``` cpp
// SystemC source for Clock Gate cell
#include "sct_assert.h"
void cgProc() {
    if (!clk_in) {
        enable = enable_in;
    }
    // To prevent error reporting for latch
    sct_assert_latch(enable);
}
void cgOutProc() {
    clk_out = enable && clk_in;
}
```

### Thread process 

Clocked thread process created with `SCT_THREAD`, `SC_THREAD` or `SC_CTHREAD` macros.
Clocked thread process in SystemC has one or multiple states specified
with `wait()`/`wait(N)` calls. Thread process normally has one reset section -- the code up to first `wait()` call. In the reset section, local variables are declared and defined, channels which are modified in the process are initialized (normally with their `reset()` functions).
Thread process main loop can contain one or more `wait()`/`wait(N)` calls. It is recommended to have only one `wait()` call in the end of the main loop, i.e. one state.

``` cpp
// Thread process with one state example
void multiStateProc() {
   sc_uint<16> x = 0;
   sig = 1;              
   wait();                    // STATE 0

   while (true) {
      sc_uint<8> y = a.read(); 
      x = y + 1;
      sig = x;           
      wait();                 // STATE 0
   }
}
```

``` cpp
// Thread process with two states example
void multiStateProc() {
   sc_uint<16> x = 0;
   sig = 1;              
   wait();                    // STATE 0

   while (true) {
      sc_uint<8> y = a.read(); 
      x = y + 1;
      wait();                 // STATE 1
      sig = x;           
   }
}
```


### Register variables in thread reset section 

There are some limitations to use register variables in reset section of
clocked thread. Register variable is a variable which keeps its values
between thread states. Register variable modification in reset section is error:

``` cpp
void proc() {
    int i = 1;      // Register variable
    i++;            // Error reported
    i += 1;         // Error reported
    wait();
    while (true) {
       out = i;     // Value from reset used here
       wait();
    }        
```


Another problem with register variable is reading it in reset section.
As soon as the variables has non-blocking assignment it values could be
incorrect in RHS of the following statements. For operations which read
register variable in reset section warning is reported:

``` cpp
void proc() {
    int i = 1;      // Register variable
    int j = i;      // Warning reported
    i = j;          // X results in SV simulation
    wait();
    while (true) {
       out = i;     // Value from reset used here
       wait();
    }        
```

### Thread process without reset

Thread process without reset supported with limitations: such process
can have only one `wait()` and cannot have any code in reset section.

``` cpp
// Clocked thread without reset
SC_CTOR(test_reset) {
   SC_CTHREAD(proc, clk.pos());
}

void proc() {
   while (true) {
      int i = 0;
      wait();
   }
}
```


## Data types

This section describes types can be used in process functions. SystemC
integer types ```sc_int```, ```sc_uint```, ```sc_bigint```, ```sc_biguint``` are
supported. C++ ```bool```, ```char```, ```short```, ```integer```, ```long integer``` and ```long long integer``` and their unsigned versions are supported. Generated
SystemVerilog data types shown in Table below.

| SC/C++ type | SV type | SC synthesizable subset |
|-------------|---------|-------------------------|
| `sct_uint<N>` | `logic [N]` | N is 0...+inf, recommended to use |
| `sct_int<N>` | `logic signed [N]` | N is 0...+inf, recommended to use |
| `sc_uint<N>` | `logic [N]` | N is 1...64 |
| `sc_biguint<N>` | `logic [N]` | N is 1...+inf |
| `sc_int<N>` | `logic signed [N]` | N is 1...64 |
| `sc_bigint<N>` | `logic signed [N]` | N is 1...+inf |
| `sc_bv<N>` | `logic [N]` | |
| `bool` | `logic` | |
| `char`, `signed char` | `logic signed [8]` | |
| `unsigned char` | `logic [8]` | |
| `short` | `logic signed [16]` | |
| `unsigned short` | `logic [16]` | |
| `int` | `integer` | 32bit |
| `unsigned int` | `integer unsigned` | 32bit |
| `long` | `logic signed [64]` | 32/64bit depends on platform |
| `long long` | `logic signed [64]` | 64bit |
| `unsigned long` | `logic [64]` | 32/64bit depends on platform |
| `unsigned long long` | `logic [64]` | 64bit |
| `__uint128_t` | `logic [128]` | |
| `__int128_t` | `logic signed [128]` | |


It is recommended to use SystemC data types instead of C++ types where
it is possible. Using C++ data types make sense for 1bit value -- `bool`
type and as `for` - loop iterators. Using `sc_bigint<N>`/`sc_biguint<N>`
results in more accurate arithmetic than `sct_int<N>`/`sct_uint<N>` and
is free from implicit promotion to `int64_t`/`uint64_t`. Drawback of
`sc_bigint<N>`/`sc_biguint<N>` is some simulation speed slow down.

There are two special types `sct_uint<N>` and `sct_int<N>` provided.
They are bit-accurate integer types which support any number of bits:

-   `sct_int<N>` is signed integer with N bits, where N is zero or
    positive,

-   `sct_uint<N>` is unsigned integer with N bits, where N is zero or
    positive.

`sct_int<N>` and `sct_uint<N>` types automatically substitute
`sc_int<N>` or `sct_bigint<N>` and `sct_uint<N>` or `sct_biguint<N>`
depends on bit width. These types are implemented in `sct_sel_types.h`.


Zero width integer types `sct_int<0>` and `sct_uint<0>` intended to
represent optional variables, signals, ports and record fields. In the
generated SV such variables are not declared, assignment to such
variable is not generated, using such variable as RValue replaced with
`0`. The same works for zero width signals, ports and record fields.

``` cpp
template <unsigned N>
struct MyModule : public sc_module {
  sct_uint<N> optVar;
  sc_in<sct_uint<N>> optInPort{"optInPort"};
  sc_signal<sct_uint<N>> optSig{"optSig"};
  struct Rec {
     sct_uint<N> optField;     
  };
}
```

Uninitialized local variable of SystemC types (sc_uint, sc_biguint,
sc_int, sc_biguint) has got zero value in generated code as it got this
value in the default constructor. That means declaration of such
variable leads to its initialization by `0`.

Not supported SystemC data types: `sc_lv`, `sc_logic`, `sc_signed`,
`sc_unsigned`, `sc_fix`, `sc_ufix`, `sc_fixed`, `sc_ufixed`. Not
supported floating point C++ data types: `float`, `double`.

C++ types can be used together with SC types. Signed and unsigned types
should never be mixed, as that can lead to unexpected result for
operations with negative values. For operations with mix of signed and
unsigned arguments of `sc_int`/`sc_uint` types SystemC simulation can
differ from generated SystemVerilog simulation. 

## Literals

SystemC literals are used to initialize local and member variables as
well as constant and static constants. ICSC supports numerical literals
represented as integer value as well as C string. Numerical literals in
C string form support more than 64bit values. Numerical literals can
have binary, octal, decimal or hexadecimal radix. Negative literal more
than 64bit can have only decimal radix.

``` cpp
int a = 42;                 // Decimal literal
sct_uint<16> b = 0xF1;      // Hexadecimal literal
sct_int<16> c  = -0xF1;     // Hexadecimal literal negative value
sct_uint<65> d = "0x1FFFFFFFFFFFFFFFF";   // Literal >64bit in C string   
sct_int<70> e  = "-36893488147419103231"; // Negative literal >64bit in C string 
```

To initialize a variable with all bits 0 or 1 there are special
templates:
* `sct_zeros<N>` is N-bit zeros literal of `sct_uint<N>` type,
* `sct_ones<N>` is N-bit ones literal of `sct_uint<N>` type.

``` cpp
// Variable declaration with 0/1 initialization
sct_uint<12> a = sct_zeros<12>;   // All zeros
sct_uint<66> b = sct_ones<66>;    // All ones
sct_int<67> c  = -sct_zeros<65>;  // Negative value
auto d         = sct_zeros<90>;   // Using auto type deduction
```

Literals are limited with 16384bit width.

## Pointers and references

It is not recommended to use any pointers.

This section describes operations with references in process functions. 
In module constructors there is no limitations on references usages. 
Reference/constant reference type members and local variables supported.
Constant reference can be initialized with variable, literal or constant
expression. Reference function parameter supported. Return value from
function by reference not supported.

``` cpp
// Reference example
template <class T>
T const_ref(const T& val) {
   T j = val+1;
   return j;
}

void refProc() {
  int a;
  int &b = a;             // Local reference
  b = 1;
  int i = const_ref(a);   // Parameter passed as reference
  i = const_ref(1);
}
```

## Collections: arrays and SC vector types

Build-in C++ arrays can be referred to as arrays. Such arrays are supported as module members and function local variables as well. One-dimensional and multidimensional
arrays are supported. Array of modules/modular interfaces and
modules/modular interfaces pointers are supported. In array of
module/modular interface base class pointers all elements must be the
same class. It is not recommended to use pointers in array or anywhere
else, but array of signals/ports and signal/port pointers are supported.
Array of port interfaces (```sc_port<IF>```) not supported. Array of records
and record pointers are supported.

``` cpp
static const unsigned N = 5;   
static const unsigned M = 10;   
sc_uint<16>             a[N];
sc_in<bool>             in[N];
sc_out<sc_uint<4>>      out[N][M];
sc_signal<int>*         sig1[N];   // Not recommended
sc_vector<sc_signal<int>> SC_NAMED(sig2, N); // Recommended array of N elements

SC_CTOR(myModule) {
  for (int i = 0; i < N; i++) {
     char sname[32];
     sprintf(sname, "sig1_%d", i); 
     sig1[i] = new sc_signal<int>(sname);
  }   
}
```

Array of any pointers must be homogeneous, all elements created with
operator new, but not pointers to existing objects. In array of base
class pointers all elements must be the same class. Array and array of
pointers, including array of signals/ports, can be a function parameter.

`std::array` can be used instead of build-in C++ array almost
everywhere. `std::array` can be assigned to another array of the same
type.

``` cpp
static const unsigned N = 5;
std::array<sc_signal<int>, N> sig;
std::array<sc_int<16>, 10> data;
void procFunc() {
   std::array<unsigned, 3> a;
   std::array<unsigned, 3> b;
   std::array<std::array<unsigned, 3>, 2> c;   
   a[0] = sig[1].read();
   b = a;
   c[0] = b;
}
```

Signal/port array index cannot be the same signal/port array. Array of
record cannot be accessed at index which is the same array element.

``` cpp
sc_in<T> a[N], b[M]; 
...
a[a[i]] = 0;       // Not supported 
a[b[i]] = 0;       // Supported
a[a[i].m].m = 0;   // Not supported 
a[b[i].n].m = 0;   // Supported
```

It is recommended to use SC vector (`sc_vector`) for signals/ports and
modules instead of arrays. SC vector is not supported for modular
interface yet. Multi-dimensional vector (vector of vectors) also
supported. SC vector instances can be instantiated in modules and
modular interfaces, including array of modular interfaces. SC vector
cannot be passed to or returned from function.

``` cpp
sc_vector<sc_in<bool>>  req{"req"};
sc_vector<sc_out<bool>> SC_NAMED(resp, 3);
sc_vector<sc_vector<sc_signal<int>>> sig2d{"sig2d", 2};
SC_CTOR(MyModule) {
  req.init(3);
  sig2d[0].init(3);
  sig2d[1].init(3);        
  ...
}
```

## Record type

This section describes usage of record which are C++ structures or
classes which are not modules and modular interfaces. Records intended
to represent set of plain data fields. Record can be module member as
well as local variables in process functions.

Records are supported with limitations. Record can have member functions
and data members. Record member functions can contain `wait()` calls.
Record can have members of C++/SC data types. Record cannot have another
record members. Record cannot have signal/port or module/modular
interface members. Record cannot have any pointer members. Record can
have array members of C++/SC data types. Record cannot have non-default
copy/move constructors and operators.

Record can have constructor, field in-place initialization and
initializer list. Record constructor can contain function calls. Record
field in-place initialization and constructor initializer list cannot
contain function calls.

Record can have one or multiple base classes. Record base class cannot
have constructor body (constructor body should be empty), but can have
initialization list and in-place initialization. Virtual functions in
records are not supported.

Array of records supported. Array of pointers to record is not
supported.

Record reference supported. Pointer to record is not supported.

Record can be passed to function by value as well as by
reference/constant reference, such record must have trivial copy
constructor. Record can be returned from function by value, such record
must have trivial copy constructor.

It is recommended to provide default (no-parameters) constructor with
default implementation. Such constructor is implicitly used for
collections of records and channels of records.

``` cpp
// Simple record example
struct Rec1 {
    int x;
    sc_int<2> y;
    Rec1() = default;       // Default constructor 
};
// Record with constructor
struct Rec2 {
    sc_uint<16> a;       
    bool b;
    Rec2() = default;       // Default constructor 
    Rec2(int i) : b(i == 42) {
        a = i + 1;
    }
};
```

Record declaration with and without parameters supported in the
following forms:

``` cpp
Rec1 r1;            // OK
Rec1 r1();          // OK
Rec1 r1{};          // OK
Rec1 r1 = f();      // OK, where f() is a function returns Rec1
Rec1 r1 = Rec1{};   // Error
Rec1 r1 = Rec1();   // Error
Rec2 r2(42);        // OK
Rec2 r2{42};        // OK
```

Record assignment supported in the following forms:

``` cpp
Rec1 r0;
Rec1 r1;
r1 = r0;            // OK
r1 = Rec1();        // OK
r1 = Rec1{};        // OK
r1 = f();           // OK, where f() is a function returns Rec1
r2 = Rec2(42);      // OK
r2 = Rec2{42};      // OK
```

### Record type in signals and ports

Signal and port of record type are supported. Record used as signal and
ports type should have a constructor without parameters. This
constructor is used for the signal/ports initialization. Such a record
should have `operator==()` and `operator<<(std::ostream)` and
`sc_trace()` defined.

``` cpp
struct SRec {
    int x;
    sc_int<2> y;
    bool operator == (const SRec& other) {
        return (x == other.x && y == other.y);
    } 
};
::std::ostream& operator << (::std::ostream& os, const SRec& s) {
    os << s.x << s.y;
    return os;
}
namespace sc_core {
    void sc_trace( sc_trace_file* , const SRec& , const std::string& ) {}
}
```

Signal and ports of record can be read/written and assigned to a record
variable.

``` cpp
sc_in<SRec>     in{"in"};
sc_out<SRec>    out{"out"};
sc_signal<SRec> s{"s"};
...
SRec r1;          // OK
r1 = in.read();   // OK
r1 = s;           // OK
s = Rec1();       // OK
s.write(r1);      // OK
out = r1;         // OK
out = Rec1{};     // OK
int x = in.read().x;   // OK
```

## Union type

Union type not supported.

## Type cast

This section describes type cast operations can be used in process
functions.

Type cast in C style ((T)x), functional style (T(x)), and static cast
supported for right side of assignment statement and function arguments.
Type cast for left side of assignment is ignored. Constant cast
`const_cast` is prohibited in left part of assignment, and ignored
elsewhere. Reinterpret and dynamic type casts are not supported.

Type cast can be used to change width or/and signness of the variable,
literal or expression. Type cast to change unsigned object to signed is
supported in binary, unary and compound operations. In other operations
type cast to signed as well as all type casts to unsigned are ignored.

Multiple casts for one object are supported. SystemC type conversion to
C++ integer methods `to_int(), to_uint()`, `to_long(), to_ulong(`),
`to_int64(), to_uint64()` supported.

``` cpp
int i;
bool b;
sc_uint<4> x;
sc_uint<8> y;
b = (bool)i; 
i = x.to_int();
y = (sc_uint<3>)x;
y = (sc_uint<6>)((sc_uint<2>)x);
```

``` {style="myverilog"}
b = |i;
i = 32'(x);
y = 3'(x);
y = 6'(2'(x));
```

Type cast to cast negative value to unsigned is prohibited. Type cast
unsigned with set high bit to signed negative is prohibited.

``` cpp
unsigned u = 0x1FFFFFFFFUL;
int i = -1;
long l = (int)u;                    // Prohibited as result is negative value
unsigned long ul = (unsigned)i;     // Prohibited as operand has negative value
```

Type cast to base class supported for function call (T::f()) and member
access (T::m).

## Dynamic memory allocation

Dynamic memory allocation is not recommended.

Dynamic memory allocation is supported at elaboration phase only, i.e. in module constructors and functions called from there. Dynamic memory allocation not supported in process functions. Dynamic allocation supported for all types including modules, interfaces, signals and ports. That is
also supported for array of pointers to modules, interfaces, signals and ports.
ICSC uses dynamic elaboration that provides arbitrary C++ code support at elaboration phase, but
not able to distinguish between pointer to dynamically allocated object and dangling pointer. To solve
this problem ICSC uses overriding operators new and new[]. For modules, interfaces, signals, ports and
other inheritors of sc_object operators new and new[] overridden in the patched SystemC library
used by ICSC.
For dynamic memory allocation for non-sc_object types, like C++ types, there are special functions
sc_new and sc_new_array. sc_new is used for scalar types instead of new, sc_new_array used for arrays instead of new[]. sc_new and sc_new_array declarations:
```cpp
template<class T, class... Args> T* sc_new(Args&&... args);
template<class T> T* sc_new_array(size_t array_size);
```

Using sc_new and sc_new_array examples:
```cpp
struct MyRec {
    int i;
    MyRec(int i_) : i(i_) {};
};

sc_signal<bool>* ap;
bool* bp;
sc_uint<8>* vp;
MyRec* mp;
sc_in<int>** ports;
sc_signal<int>* signals;

SC_MODULE(MyModule) {    
    ap = new sc_signal<bool>("a"); // OK, signal is sc_object
    bp = new bool; // ERROR, non-sc_object
    bp = sc_new<bool>(); // OK
    vp = new sc_uint<8>(); // ERROR, non-sc_object
    vp = sc_new<sc_uint<8>>(); // OK
    mp = new MyRec(42); // ERROR, new for non-sc_object
    mp = sc_new<MyRec>(42); // OK, using sc_new
    ports = new sc_in<int>* [10]; // ERROR, new for pointer, non-sc_object
    ports = sc_new_array<sc_in<int>*>(10); // OK, using sc_new_array
    signals = new sc_signal <int>[10]; // OK, array of sc_objects
}
```

## Control flow operators

All control flow operators are supported. Conditions of `if`, `?`,
`for`, `while`, `do..while` should be expression without side effects.
Complex conditions with `||`, `&&`, `!` and brackets supported. If left
part of logical expressions with `||` and `&&` evaluated as constant,
right part code is not generated. That allows to check pointer is not
null and do the pointer de-reference it in the condition expression.
`if` and `?` conditions, including complex conditions, can contain
function call without side effects and without `wait()`. `for`, `while`,
`do..while` conditions cannot have any function call.

There are two kind of synthesizable loops:

1.  loop without `wait()/wait(N)`, for these loops iteration number must
    be statically determinable,

2.  loop with `wait()/wait(N)`, for these loops iteration number may be
    unknown.

### if

`if` statement is translated into SystemVerilog `if`.

``` cpp
// Operator if examples
if (a || b) {...}
if (true || a) {...}
if (false && b) {...}
```

### switch

`switch` statement is translated into SystemVerilog `case`, see
`switch` statement can have one or more cases including optional `default` case. Each case must have one and only one final `break` as the last statement of the case. `default` case also must contain final `break`. Another option is empty case or empty
`default` case. For empty case the next non-empty case (possibly
`default` case) code is copied in the generated SV.

`switch` case code can contain if/loop statements as well as inner
switch statements. `switch` case code can contain function calls.
`switch` statement in called function can contains `return` statements
in the end of all cases. For such `switch` cases final `break` statement
not allowed, no mix of `return` and `break` supported.

``` cpp
// Operator switch example
switch (i) {
case 0: i++; break;
case 1: i--; break;
default: i = 0; break;
}
```

`switch` statement in called function can contain `return` statements in
the end of all cases. For such `switch` no `break` statements required.

``` cpp
// Operator switch in function example
void f() {
    ...
    switch (i) {
    case 0: i++; return;
    case 1: i--; return;
    }
    return;
}
```

``` cpp
// Operator switch with empty case
switch (i) {
case 0: 
case 1: k = 1; break;
default: k = 2; break;
}
```

### for

`for` loop can have only one counter variable with optional
initialization, condition and increment sections. The variable can be
declared in the loop initialization. Initialization section can have
simple variable initialization or assignment only, cannot have function
call. Condition section can have one comparison operator for the loop
variable, cannot have function call. Increment section can have
increment or decrement of the loop variable, cannot have function call.

Several examples of correct `for` loop is given in the following
listing:

``` cpp
// Operator for examples
const unsigned N = 10;
for (int i = 0; i < N; i++) {...}

int i = N;
for (; i != 0; --i) {...}

int j = 0;
for (; j < N; ) {...}
```

### while

`while` condition is an arbitrary
expression without function call. Several examples of correct `while`
loop is given in the following listing:

``` cpp
// Operator while examples
const unsigned N = 10;
int i = 0;
while (i < N) {...}

int j = N;
int k = 0;
while (j != 0 && j != k) {...}

// Waiting for enable, this while loop should contain wait() at each path
while (!enable.read()) {...}
```

### do\...while

`do..while` condition is an arbitrary
expression without function call. Several examples of correct
`do..while` loop is given in the following listing:

``` cpp
// Operator do..while examples
const unsigned N = 10;
int i = 0;
do {
   ...
} while (i < N);

int j = N;
int k = 0;
do {
   ...
} while (j != 0 && j != k);
```

### break and continue

`break` statement is supported. `continue` statement is supported. It is not recommended to use `break` or `continue` in main loop as well as in any loop with `wait()` statement.


### Loops with wait 

Loop with `wait()/wait(N)` call are supported.

``` cpp
void loopProc() {
    enable = 0;
    wait();                     // STATE 0
    while (true) {
        for (int i = 0; i < 3; ++i) {
            enable = 0;
            wait();             // STATE 1
        }
        enable = 1;
    }
} 
```

A loop with `wait()/wait(N)` can contain `break` or `continue` statements.

``` cpp
void breakProc() {
    ready = 0;
    wait();                     // STATE 0
    while (true) {
        wait();                 // STATE 1
        while (!enable) {
            if (stop) break;
            ready = 1;
            wait();             // STATE 2
        }
        ready = 0;
    }
}    
```


### goto

`goto` is not supported.


## Arithmetical operations

SystemC/C++ type promotion rules differ from each other and from SV
rules. For literal and non-literal terms signed-to-unsigned and
unsigned-to-signed implicit cast detected and used as base for sign in
SV.

### Signed and unsigned literals

SystemC/C++ literals translated into SV literals with the following
rules:

-   Zero not casted.
-   C++ literal has integer type and represented in simple form is signed.
-   C++ literal with suffix U represented in based form is unsigned.
-   SC literal represented in based form and is signed or unsigned depends on its type.
-   All negative literals are signed.

In binary operators if first argument is signed type non-literal and
second is no cast literal, second argument is casted to signed literal.

### Signed and unsigned types

The following rules based on C++ type promotion rules and SC types
operators implementation. General idea is non-literal mix of signed and
unsigned considered as signed and unsigned operand converted to signed
in SV with `signed’`. If signed literal mixed with unsigned non-literal,
that considered as unsigned arithmetic, no signed cast in SV.

`enum` types can be signed and unsigned as well. There is implicit cast
to `int`, so unsinged `enum` casted to signed, no special rule required.
There are several rules for non-literals in binary operators (+, -, \*,
/, &, \^, \|, %):

-   If first argument has signed-to-unsigned cast or signed expression
    and second has no cast and is not signed type or signed expression,
    second argument is casted to signed.

-   If first argument of `sc_bigint` type and second is not signed type
    or signed expression, second argument is casted to signed.

-   If first argument of `sc_biguint` type and second is signed type,
    first argument is casted to signed.

Mixing negative signed operand and `unsigned/sc_uint` operand can
provide incorrect result, so warning is reported. This operations work
well with `sc_biguint`.

There is rule for non-literals in unary operators, if argument of
`sc_biguint` type in unary minus, the argument is casted to signed.

### Unsigned operation overflow

Operations with SC unsigned type arguments (`sc_uint` and `sc_biguint`)
can lead to overflow. Using such expressions as part of another
expression can lead to non-equivalent simulation results. It is
recommended to explicitly cast such expressions to desired width. Left
shift for C++ data types is cyclic which is not supported in the
generated SV code. Therefore left shift overflow for C++ data type is
prohibited.

``` cpp
sc_uint<7> k, m;
k = 41; m = 42;
sc_uint<8> res = (k - m) % 11;   // Non-equivalent
res = sc_uint<8>(k - m) % 11;    // OK
unsigned u = 1;
res = u << 32;                   // Non-equivalent: 1 in SC, 0 in SV
```

### Type cast in assignment

Explicit type cast can be used to narrow/widen the argument. It
translated into SV type cast. Multiple type casts are also possible:
internal one narrows value, external one extend type width, which may be
required for concatenation.

Implicit type cast, including SC data type constructor, not translated
to SV. SV has implicit narrowing of argument, for example:

``` cpp
sc_uint<3> i;
sc_uint<2> k = i;
bool b = k;
```

``` {style="myverilog"}
// Generated SystemVerilog
logic [2:0] i;
logic [1:0] k;
logic b;
k = i;     // All that work fine in ICSC, no warnings
b = k;     
```

Explicit cast can be combined with signed cast. In this case signed cast
extend data width by 1 only if required (signed changed after explicit
cast):

``` cpp
sc_int<6> x;
sc_uint<4> ux;
z = x + ux;
z = x + sc_uint<6>(ux);
z = x + sc_int<7>(ux);
```

``` {style="myverilog"}
// Generated SystemVerilog
z = x + signed'({1'b0, ux});
z = x + 6'(ux);
z = x + signed'(7'(ux));
```

### Operator comma

Operator `,` is applied to concatenate two SC integers with specified
length. For C++ integers and results of operation explicit type
conversion is required.

``` cpp
sc_uint<N> i; 
sc_uint<M> j; 
sc_uint<N+M> k;
k = (i, j);                  // OK 
k = (i, sc_uint<M>(j));      // OK, extra type conversion
k = (i, i*j);                // Error reported, type conversion required
k = (i, sc_uint<M>(i*j));    // OK
```


## Function calls

This section describes functions and function calls rules.
Module/modular interface/record static and non-static functions
supported. Global/namespace functions supported. Recursive functions not
supported.

Function can have parameters and returned value. Function can have local
variable of non-channel type. Local variables can be non-static or
constant static. No static non-constant local variables allowed.

Function parameters can be passed by value, by reference reference. Constant reference parameter argument can be literal of the corresponding type. Function can return
result by value only. Return by reference or by pointer not supported.
Function with return type `void` can use `return` statement without
argument.

``` cpp
// No parameters function
void f1() {
   m = m + 1;
}       
// Parameters passed by value 
int f2(int i, bool b = false) {
  return (b) ? i : i+1;
}    
// Parameters passed by reference 
void f3(int& i) {
  i++;
}
// Array passed 
int f5(int arr_par[3]) {
  int res = 0;
  for (int i = 0; i < 3; i++) {
    res += arr_par[i];
  }
  return res;
}
```

Function with multiple `return` supported with limitations: function must have no code after `return`. It is not recommended to use multiple `return` in a function.

In particular, `return` statement in loop is not supported. Function with return type void can have `return` statement(s) without argument. For
`return` statement without argument no code is generated.

``` cpp
// Multiple returns
int f6(int& val) {
  if (val) {
     return 1;
  } else {
     return 2;
  }
}
// Multiple returns in switch
unsigned f7(unsigned val) {
   switch (val) {
     case 1: return 1;
     case 2: return 2;
     default: return 3;
   }
}
```

Module member function can access this module fields/functions and child
modular interface instance(s) fields/functions. Access to child modular
interface instance members allowed through a port interface
(```sc_port<IF>```) or a pointer to the modular interface. The accessed
modular interface is flatten in module/modular interface with does
access to it.

Virtual functions supported. Function overload and hide function in
child class supported.

## Naming restrictions

Prefixes `sct_` and `SCT_` are used for special function and cannot be
used in user SystemC code. Suffix `_next` is used for register
variables, so it is not recommended to use such suffix for SystemC
variables.

ICSC tool provides `__SC_TOOL__` define for input SystemC project
translation. Module/interface field `__SC_TOOL_MODULE_NAME__` is
reserved for vendor memory name. Module/interface field
`__SC_TOOL_VERILOG_MOD__` is reserved for disable module generation in
SystemVerilog code.


## Logging and others

SystemC logging have multiple options including standard output stream
`std::cout`, `printf` function and `sc_trace` function.

These debugging functions are allowed in the code for synthesis.
`std::cout` together with `operator <<` are ignored as well as
`sc_trace` function. For `printf` function an equivalent `$display`
macro is generated in SV code. `printf` should have format string
parameter and one or more variables to print. For SystemC type variables
conversion to C++ types should be applied.

``` cpp
void printfProc() {
    int i;
    sc_uint<16> j;
    sc_biguint<65> bj;
    ...
    printf("i = %d, j = %d, bj = %d\n", i, j.to_uint64(), bj.to_uint64());
}
```



# SystemVerilog intrinsics

This section describe how to insert SystemVerilog intrinsic ("black box") module.

ICSC supports replacement a SystemC module with given SystemVerilog
intrinsic module. In this case no parsing of the SystemC module is
performed, so this module can contain non-synthesizable code. To replace
SystemC module it needs to define `__SC_TOOL_VERILOG_MOD__` variable of
`std::string` type in the module body. `__SC_TOOL_VERILOG_MOD__` value
can be specified in place or in the module constructor.

There are two common usages:

1.  Replace with given SystemVerilog module: `__SC_TOOL_VERILOG_MOD__`
    contains SV module code or `#include` directive;
2.  Do not generate module at all: `__SC_TOOL_VERILOG_MOD__` is empty
    string.

In second case SystemVerilog module implementation needs to be provided
in an external file.

``` cpp
struct my_register : sc_module {
  std::string __SC_TOOL_VERILOG_MOD__[] = R"(
     module my_register (
        input  logic [31:0] din,
        output logic [31:0] dout
     );
     assign dout = din;
     endmodule)";

  SC_CTOR (my_register) {...}
  ...
}
```

``` cpp
// SystemVerilog generated
// Verilog intrinsic for module: my_register 
module my_register (
    input  logic [31:0] din,
    output logic [31:0] dout
);
assign dout = din;
endmodule
```


# Memory module

Memory module is intended to instantiate memory hard IP (HIP) in the design code.
Memory module is implemented as set of classes includes common interface for memory access and wrapper implementations for RF, SRAM and ROM for different technology processes.

Common interface for memory access:

``` cpp
template <unsigned WORD_COUNT,  /// Number of words in the memory (depth)
          unsigned DATA_WIDTH,  /// Data width
          bool     USE_BENABLE  /// Use bit write enable pins
          >
class mem_wrapper_if : public virtual sc_interface {
    typedef sc_uint<ADDR_WIDTH>              Addr_t;  // word addressing
    typedef typename sc_suint<DATA_WIDTH>::T Data_t;

    //--- Data interface
    /// Is read data valid
    virtual bool isDataValid() = 0;

    /// Get read data, asserts that data is valid when checkValid == true
    virtual Data_t getData(bool checkValid = true) = 0;

    /// Add read data and data valid to sensitivity list
    virtual void addRDataSensitive(sc_sensitive &s) = 0;

    /// Read memory request (set read request and address)
    /// get read data on the following cycle using getReadData()/isReadValid()
    virtual void readRequest(Addr_t addr, bool en = true) = 0;

    /// Write memory request (set write request, address, data, and bit enable)
    virtual void writeRequest(Addr_t addr, Data_t data, Data_t bitEnable,
                              bool en = true) = 0;

    /// Write memory request (set write request, address, data)
    virtual void writeRequest(Addr_t addr, Data_t data, bool en = true) = 0;

    //--- Memory specific static configuration pins control
    /// Reset memory specific configuration pins to default values
    virtual void resetMemcfg() = 0;

    /// Set memory specific configuration pins
    virtual void setMemcfg(unsigned mode) = 0;

    /// Get memory specific configuration pins
    virtual unsigned getMemcfg() = 0;

    //--- Memory power control
    /// Sleep memory in various modes: lite sleep (0), deep sleep (1), ...
    /// Note: ensure no active memory operations happen
    virtual void sleepMemory(unsigned mode = 0) = 0;

    /// Wakeup memory from sleep
    /// Note: check isMemPwrReady() before any memory operations
    virtual void wakeupMemory() = 0;

    /// Power-off memory
    /// Note: ensure no active memory operations happen
    virtual void pwroffMemory() = 0;

    /// Power-on memory
    /// Note: check isMemPwrReady() before any memory operations
    virtual void pwronMemory() = 0;

    /// Set memory isolation (1 - enable isolation, 0 - normal operation)
    virtual void setMemoryIsolation(unsigned mode) = 0;

    /// Returns true if memory is powered on and active (not sleeping)
    virtual bool isMemoryPowerReady() = 0;

    /// Add memory power ready signal to sensitivity list
    virtual void addMemPwrRdySensitive(sc_sensitive &s) = 0;
}
```

P1222 RF memory instantiation with 16 words, 8 bit data width and bit
enable example:

``` cpp
/// Memory Wrapper for p1222 RF ip222rfsbhpm1r1w16x8be8m4p3
class ip222rf_16x8be : public rf_wrapper<16, 8, 1> {
public:
    explicit ip222rf_16x8be(const sc_module_name& name, MwPwrCtl pcMode,
                            MwMemcfgCtl mcMode)
        : rf_wrapper<16, 8, 1>(name, "ip222rfsbhpm1r1w16x8be8m4p3", pcMode,
                               mcMode) 
    {}
};
// Memory module instantiation
ip222rf_16x8be rfmem{"rfmem", MwPwrCtl::Internal, MwMemcfgCtl::Internal}};
```

## Memory configurator

Memory configurator intended to assemble memory module with required
data width and word number with multiple memory modules. Memory
configurator consists of several classes provides data width and word
number assembling.

Data width assemble with multiple memories is provided by
`mem_wrapper_wconcat` and `mem_wrapper_wconcat_n` classes.
`mem_wrapper_wconcat` allows to join two memory modules with the same
word number to get data width as sum of the module data width.
`mem_wrapper_wconcat_n` allows to join specified number of memory module
to multiply data width.

``` cpp
typedef mem_wrapper_wconcat<ip222rf_16x8be, ip222rf_16x8be> ip222rf_16x16be;
typedef mem_wrapper_wconcat_n<ip222rf_16x8be, 3> ip222rf_16x24be;
```

Word number assemble with multiple memories is provided by
`mem_wrapper_dconcat` and `mem_wrapper_dconcat_n` classes.
`mem_wrapper_dconcat` allows to join two memory modules with the same
data width to get word number as sum of the module data width.
`mem_wrapper_dconcat_n` allows to join specified number of memory module
to multiply word number.

``` cpp
typedef mem_wrapper_dconcat<ip222rf_16x16be, ip222rf_16x16_be> ip222rf_32x16be;
typedef mem_wrapper_dconcat_n<ip222rf_16x16be, 5> ip222rf_80x16be;
```

Memory class instantiation example:

``` cpp
mem_wrapper_dconcat<ip222rf_80x16be, ip222rf_16x16be> rfmem{
        "rfmem", MwPwrCtl::Internal, MwMemcfgCtl::Internal};
```


## Memory module name

This section describes how to create a custom memory module with module
name specified.

To support vendor memory it needs to specify memory module name at
instantiation point and exclude the SV module code generation (memory
module is external one). To exclude SV module code generation empty
`__SC_TOOL_VERILOG_MOD__` should be used. To specify memory module name
it needs to define `__SC_TOOL_MODULE_NAME__` variable in the module body
and initialize it with required name string.

If there are two instances of the same SystemC module, it is possible to
give them different names, but `__SC_TOOL_VERILOG_MOD__` must be
declared in the module. If `__SC_TOOL_VERILOG_MOD__` is not declared the
SystemC module, only one SV module with first given name will be
generated .

Module name could be specified for module with non-empty
`__SC_TOOL_VERILOG_MOD__`, but module names in `__SC_TOOL_MODULE_NAME__`
and `__SC_TOOL_VERILOG_MOD__` should be the same.

If specified module name in module without `__SC_TOOL_VERILOG_MOD__`
declaration conflicts with another module name, it updated with numeric
suffix. Specified name in module with `__SC_TOOL_VERILOG_MOD__`
declaration never changed, so name uniqueness should be checked by user.

``` cpp
// Memory stub example
struct memory_stub : sc_module {
    // Disable Verilog module generation
    std::string __SC_TOOL_VERILOG_MOD__[] = "";  
    // Specify module name at instantiation
    std::string __SC_TOOL_MODULE_NAME__;             
    explicit memory_stub(const sc_module_name& name,
                         const char* verilogName = "") :
        __SC_TOOL_MODULE_NAME__(verilogName)
    {}
};

// Memory instance at some module
memory_stub  stubInst1{"stubInst1", "pxxxrf256x32ben"};
memory_stub  stubInst2{"stubInst2", "pxxxsram1024x32ben"};
memory_stub  stubInst3{"stubInst3"};
stubInst1.clk(clk); 
stubInst2.clk(clk); 
stubInst3.clk(clk); 
...
```

``` cpp
// SystemVerilog generated 
pxxxrf256x32ben     stubInst1(.clk(clk), ...);
pxxxsram1024x32ben  stubInst2(.clk(clk), ...);
memory_stub         stubInst3(.clk(clk), ...);
```













