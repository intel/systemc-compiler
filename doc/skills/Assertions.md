## Immediate assertions

There are several types of C++, SystemC, and ICSC assertions to use in design verification:

* ```assert(expr)``` -- general C++ assertion, in case of violation leads to abort SystemC simulation, ignored by ICSC;
* ```sc_assert(expr)``` -– SystemC assertion, leads to report fatal error (```SC_REPORT_FATAL```), ignored by ICSC;
* ```sct_assert(expr [, msg = ""])``` -– ICSC assertion, in simulation has the same behavior as ```assert```, SVA generates System Verilog assertion (SVA) for it. Second parameter ```const char* msg``` is optional, contains message to print in simulation and used in SVA error message;

Immediate assertions are declared in ```sct_assert.h``` (```components/common/sct_assert.h```). This assertion can be used for SystemC simulation ans well as for generated Verilog simulation. In generated Verilog there is equivalent SVA ```assert``` with error message if specified. Error message should be string literal (```const char*```).

```cpp
// SystemC source
#include "sct_assert.h"
void sct_assert_method()  {
   sct_assert(cntr == 1);
   sct_assert(!enable, "User error message");
}
```

Using ```sct_assert``` is more complicated than temporal assertion ```SCT_ASSERT``` in module scope or ```SCT_ASSERT_THREAD``` in thread process as described below. So, it is strongly recommended to use ```SCT_ASSERT(expr, clk.pos())``` or ```SCT_ASSERT_THREAD(expr, clk.pos())``` instead of ```sct_assert(expr)```.

## Temporal assertions

Temporal assertions in SystemC intended to be used for advanced verification of design properties with specified delays. These assertions looks similar to System Verilog assertions (SVA). The assertions can be added in SystemC design in module scope and clocked thread process:

```cpp 
SCT_ASSERT(EXPR, EVENT);                       // In module scope 
SCT_ASSERT(LHS, TIME, RHS, EVENT);             // In module scope 
SCT_ASSERT_STABLE(LHS, TIME, RHS, EVENT);      // In module scope 
SCT_ASSERT_ROSE(LHS, TIME, RHS, EVENT);        // In module scope 
SCT_ASSERT_FELL(LHS, TIME, RHS, EVENT);        // In module scope 
SCT_ASSERT_THREAD(EXPR, EVENT);                // In clocked thread 
SCT_ASSERT_THREAD(LHS, TIME, RHS, EVENT);      // In clocked thread 
SCT_ASSERT_LOOP(LHS, TIME, RHS, EVENT, ITER);  // In for-loop inside of clocked thread
```
These ways are complementary. Assertions in module scope avoids polluting process code. Assertions in clock thread allows to use member and local variables. Assertions in loop can access channel and port arrays.

Temporal assertions in module scope and clocked thread have the same parameters:

* ```EXPR``` -- assertion expression, checked to be true,
* ```LHS``` -- antecedent assertion expression which is pre-condition, 
* ```TIME``` -- temporal condition is specific number of cycles or cycle interval,
* ```RHS``` -- consequent assertion expression, checked to be true if antecedent expression was true in past,
* ```EVENT``` -- cycle event which is clock positive, negative or both edges.
* ```ITER``` -- loop iteration counter variable(s) in arbitrary order.

If ```clk``` is clock input, then EVENT specified with ```clk.pos()```, ```tt clk.neg()``` or ```clk``` correspondingly.

Assertion expression can be arithmetical or logical expression, with zero, one or several operands. Assertion expression cannot contain function call and ternary operator ```?```.

Temporal condition specified with:
```cpp
SCT_TIME(TIME);              // time delay, TIME is number of cycles,
SCT_TIME(LO_TIME, HI_TIME);  // time interval, LO_TIME and HI_TIME are number of cycles.
```
Temporal condition specifies time delay when ```RHS``` checked after ```LHS``` is true. Temporal condition is number of cycles or cycle interval, where cycle is clock period. For cycle interval ```RHS``` should be true at least at one of the cycles.
Specific number of cycles is integer non-negative number. Cycle interval has low time and high time. Low time and high time can be the same. There is reduced form of time condition with brackets only ```(TIME) or (LO_TIME, HI_TIME)``` (see examples below).

Temporal assertions are declared in ```sct_assert.h``` (```include/cstcommon/sct_assert.h```), it needs to be included. 

To disable temporal assertions macro ```SCT_ASSERT_OFF``` should be defined. That can be required to use another HLS tools which does not support these assertions.
To avoid SVA assertion generating ```NO_SVA_GENERATE``` option of ```svc_target``` should be used. 

### Temporal assertions in module scope

Temporal assertions in module scope added with 

```cpp 
SCT_ASSERT(EXPR, EVENT);
SCT_ASSERT (LHS, TIME, RHS, EVENT);
SCT_ASSERT_STABLE(LHS, TIME, RHS, EVENT);      
SCT_ASSERT_ROSE(LHS, TIME, RHS, EVENT);        
SCT_ASSERT_FELL(LHS, TIME, RHS, EVENT);        
``` 

Time delay for these assertion means immediate or one cycle delayed checking of stable/rose/fell of the consequent expression. Time interval for ```SCT_ASSERT_STABLE``` specify how long the consequent expression should be stable.

```SCT_ASSERT_STABLE```, ```SCT_ASSERT_ROSE``` and ```SCT_ASSERT_FELL``` have some limitation on time parameter:
 * ```SCT_ASSERT_STABLE``` can have time delay ```0``` and ```1``` and time interval ```(0, 1)```,
 * ```SCT_ASSERT_ROSE``` and ```SCT_ASSERT_FELL``` can have time delay ```0``` and ```1```only.

Assertion expression can operate with signals, ports, template parameters, constants and literals. Non-constant member data variables (not signals/ports) access in assertion can lead to data races. Because of that only member data which has stable value after elaboration phase could be used in assertions. 

There are several examples:
```cpp
static const unsigned N = 3;
sc_in<bool> req;
sc_out<bool> resp;
sc_signal<sc_uint<8>> val;
sc_signal<sc_uint<8>>* pval;
int m;
sc_uint<16> arr[N];
...
77: SCT_ASSERT(req || val == 0, clk.pos());             // OK
78: SCT_ASSERT(req, SCT_TIME(1), resp, clk.pos());      // OK
79: SCT_ASSERT(req, SCT_TIME(N+1), resp, clk.neg());    // OK, constant time
80: SCT_ASSERT(req, (2), val.read(), clk);              // OK, brackets only form
81: SCT_ASSERT(val, SCT_TIME(2,3), *pval, clk.pos());   // OK, time interval
82: SCT_ASSERT(arr[0], (N,2*N), arr[N-1], clk.pos());   // OK, brackets only form
83: SCT_ASSERT(val == N, SCT_TIME(1), resp, clk.pos()); // OK, constant used
84: SCT_ASSERT(m == 0, (1), resp, clk.pos());           // Error, member variable used
85: SCT_ASSERT(resp, (0,2), arr[m+1], clk.pos());       // Error, non-constant index
86: SCT_ASSERT_STABLE(req, (0), resp, clk.pos());       // OK
87: SCT_ASSERT_STABLE(req, (2), resp, clk.pos());       // Error, delay can be 0 or 1
88: SCT_ASSERT_STABLE(req, (1,3), resp, clk.pos());     // OK
89: SCT_ASSERT_ROSE(req, (0), resp, clk.pos());         // OK  
90: SCT_ASSERT_FELL(req, (1), resp, clk.pos());         // OK  
91: SCT_ASSERT_ROSE(req, (0,1), resp, clk.pos());       // Error, time interval for rose
```

Assertion expression can operate with SingleSource library channels. Target method ```request()```, initiator method ```ready()``` and FIFO methods ```request()```, ```ready()```, ```size()```, ```elem_num()``` could be used. 
Assertion expression can contain call of functions which have no parameters, have integral return type and consists of only return statement.

Single source channels examples:

```cpp
sct_target<int>                 targ{"targ"};
sct_initiator<int>              init{"init"};
sct_fifo<sc_uint<8>, 3>         fifo{"fifo"};
...
93: SCT_ASSERT(fifo.request(), (1), fifo.ready(), clk.pos());  
94: SCT_ASSERT(init.ready(), (1), !init.ready() && targ.request(), clk.pos());  
```

### Temporal assertions in clocked thread process

Temporal assertions in clocked thread added with 
```cpp
SCT_ASSERT_THREAD(EXPR, EVENT);                
SCT_ASSERT_THREAD(LHS, TIME, RHS, EVENT);      
```

These assertions can operate with local data variables and local/member constants. Non-constant member data variables (not signals/ports) access in assertion can lead to data races. Because of that only member data which has stable value after elaboration phase could be used in assertions. Thread process assertions have **no advantages** over module scope assertions, so modules scope assertions are recommended to use.

These assertions can operate with SingleSource library channels and can contain calls of simple functions like assertions in module scope.

Assertion in thread process can be placed in reset section (before first ```wait()```) or after reset section before main infinite loop. Assertions in main loop not supported. Assertions can be placed in ```if``` branch scopes, but this ```if``` must have statically evaluated condition. Variable condition of assertion should be considered in its antecedent (left) expression. 

```cpp
void thread_proc() {
   // Reset section
   ...
   SCT_ASSERT_THREAD(req, SCT_TIME(1), ready, clk.pos());     // Assertions in reset section
   wait();                        
   SCT_ASSERT_THREAD(req, SCT_TIME(2,3), resp, clk.pos());    // Assertions after reset section

   // Main loop 
   while (true) { 
      ...                                   // No assertion in main loop 
      wait();
}}
```


There an example with several assertions:

```cpp
static const unsigned N = 3;
sc_in<bool> req;
sc_out<bool> resp;
sc_signal<bool> resp;
sc_uint<8> m;
...
void thread_proc() {
   int i = 0;
   SCT_ASSERT_THREAD(req, SCT_TIME(0), ready, clk.pos());        // OK
   SCT_ASSERT_THREAD(req, SCT_TIME(N+1), ready, clk.pos());      // OK, constant in time parameter
   SCT_ASSERT_THREAD(req, (2,3), i == 0, clk.pos());             // OK, local variable used
   wait();
   if (N > 1) {
       SCT_ASSERT_THREAD(req, SCT_TIME(1), resp, clk.pos());     // OK, statically evaluated condition
   }
   SCT_ASSERT_THREAD(m > 1, (2), ready, clk.pos());              // OK, member variable used
   while (true) {   
      ...
      SCT_ASSERT_THREAD(req, SCT_TIME(0), ready, clk.pos());     // Error, assertion in main loop
      wait();
}}
```

### Temporal assertions in loop inside of clocked thread

Temporal assertions in loop inside of clocked thread added with 
```cpp
SCT_ASSERT_LOOP (LHS, TIME, RHS, EVENT, ITER);
```
```ITER``` parameter is loop variable name or multiple names separated by comma.

Loop with assertions can be in reset section or after reset section before main infinite loop. The loop should be ```for```-loop with statically determined number of iteration and one counter variable. Such loop cannot have ```wait()``` in its body. 

```cpp
void thread_proc() {
   // Reset section
   ...
   for (int i = 0; i < N; ++i) {
      SCT_ASSERT_LOOP(req[i], SCT_TIME(1), ready[i], clk.pos(), i);
      for (int j = 0; j < M; ++j) {
         SCT_ASSERT_LOOP(req[i][j], SCT_TIME(2), resp[i][N-j+1], clk.pos(), i, j);
   }}
   wait();                        
   while (true) { 
      ...                        // No assertion in main loop 
      wait();
}}
```

## Special assertions

There are special assertions mostly intended for tool developers.

* ```sct_assert_latch(var [, latch = true])``` -- assert that given variable, signal or port is latch if second parameter is true (by default), or not latch otherwise. Latch object is defined only at some paths of method process.
* ```sct_assert_const(expr)``` -- check given expression is true in constant propagation analysis
* ```sct_assert_level(level)``` -- check current block level with given one
* ```sct_assert_unknown(value)``` -- check give value is unknown, i.e. not statically evaluated
* ```sct_assert_defined(expr)``` -- check given expression  is defined 
* ```sct_assert_read(expr)``` -- check given expression is read 
* ```sct_assert_register(expr)``` -- check given expression is read before defined 
* ```sct_assert_array_defined(expr)``` -- check given expression is array and some element is defined at least on some paths

### Latch assertion usage 

Normally ICSC does not allow to have latch in SystemC source, but there are some cases where latch is required. There is ICSC assertion ```sct_assert_latch``` which intended to specify latch in method process. It suppresses ICSC error message for latch variable, signal or port. 

Normal method process translated into ```always_comb``` block in SystemVerilog. For method with latch ```always_latch``` block is generated.

```cpp
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
