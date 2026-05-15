# SystemC supported

## Terminology
* Module -- SystemC module, a C++ class/structure inherits ```sc_module```
* Modular interface -- SystemC module which inherits ```sc_interface```
* Record -- C++ class/structure which is not module nor modular interface
* Signal -- SystemC ```sc_signal```
* Port -- SystemC ```sc_in``` and ```sc_out```
* Port interface -- ```sc_port<IF>``` connected to module/modular interface which implements ```IF```
* SC vector -- SystemC ```sc_vector```, can contain inheritors of SystemC ```sc_object```.

## Module constructor requirements
1. Dynamic allocation requirements. Allocation using ```new``` is supported for ```sc_objects``` (ports, signals, modules). For all other cases new is not supported, use use ```sc_new``` for single object and ```sc_new_array``` for array. Allocation with ```sc_new``` and ```sc_new_array``` supports all usage cases of built-in ```new```.
1. Unique owning pointer for heap-allocated objects. Two pointers to same object are not supported and leads to incorrect code generated.
1. Non-rectangular data structures are not supported, with exception for arrays of modules. Rectangular data structures means all the pointers in array points to sub-array of the same size.
1. All ports must be connected to signals in the design or its testbench. If a port not connected to a signal in the design (connected in testbench), such a port is promoted to design top module port which has the correspondent name. In case of name collision, numerical suffix is used (suffix starts with 0).
1. Port can be connected to a signal through module hierarchy. That means the port and signal can belong to different modules, not necessary to child and parent or two children of the same parent.
Example of port cross-binding:
```cpp
SC_MODULE(A) {
    sc_in<T>     SC_NAMED(i);     
};
SC_MODULE(B) {
    A            SC_NAMED(mod_a);
};
SC_MODULE(C) {
    sc_signal<T> SC_NAMED(s);
    B            SC_NAMED(mod_b);
    SC_CTOR(C) {
       mod_b.mod_a.i(s);   // Cross-binding of port "i" to signal "s"
    } 
};
```

1. Signals (sc_signal) and ports (sc_in/sc_out) cannot have a record as type parameter (will be supported soon).


## Process code requirements
1. Module is C++ class or structure inherits ```sc_module```. Module can have data members, signal/port members, and module/modular interface/record members. Module can have method and clocked thread processes. Module can have process functions as well as other member functions.

1. Modular interface is module which inherits ```sc_interface```. Modular interface has all module features, but in generated SV it is flattened into its parent module. That gives more flexibility in using modular interface, including access its members and call member functions from parent module and peer modular interfaces.

1. Any module/modular interface must have at least one constructor. Module/modular interface constructor must have ```sc_module_name``` parameter which can be passed to ```sc_module``` inheritor or just not used. Module/modular interface constructor can contains arbitrary C++ code. 

1. Record is C++ (template)class/structure which is not SystemC module or modular interface. Record can be member of module/modular interface as well as local variables in process functions. Records intended to represent set of plain data fields.

1. Records are supported with limitations. Record can have member functions and data members. Record member functions can contain ```wait()``` calls. Record can have members of C++/SC data types. Record cannot have another record members. Record cannot have signal/port or module/modular interface members. Record cannot have any pointer or reference members. Record can have array members of C++/SC data types. Record cannot have array members of record. 

1. Record can have constructor, field in-place initialization and initializer list. Record constructor can contain function calls. Record field in-place initialization and constructor initializer list cannot contain function calls.
Record cannot have non-default copy/move constructors and operators. Record can have user defined ```operator ==()``` which should have default semantic (required for record used as channel data type).

1. Record can be constructed or assigned with another record (```Rec r1; Rec r2 = r1; r2 = r1;```) or temporal record object returned from function (```Rec r1 = f(); r1 = g();```) or constructed in-place (```Rec r1 = Rec{}; r1 = Rec{}```). To use in-place initialization the record class should have default constructor with empty body, fields in-place initialization and initializer list allowed.

1. Record can have one or multiple base classes. Record base class cannot have constructor body (constructor body should be empty), but can have initialization list and in-place initialization. Virtual functions in records are not supported.

1. Channel/port can have record as its payload. Such record should have default constructor, ```operator ==()``` with default semantic, ```operator <<()``` and ```sc_trace()``` implemented. Record channel/port can be in left part of assignment if right part has: record variable, record default constructor, function call returns record, another channel/port variable of the same type. Record with base class can be used as channel payload. Signal/port with record example:
```cpp
struct Simple {
   bool a;
   sc_uint<10> b;
};
sc_signal<Simple> sig{"sig"};
...
Simple s = sig.read();
sig.write(s);
bool a = sig.read().a;
```

1. There is one top module in design. Top module can contains module instances as well as other fields. All instances are module fields, no global/namespace variables supported. Global/namespace constants supported.

1. Module/modular interface/record non-static data members and static constant data members supported. Static non-constant data members including arrays  allowed, but limited to access in one process. 
Constant static one and multi-dimensional arrays are supported: ```static constexpr``` member array with in-place initialization; ```const``` non-static member and global array with in-place initialization. Static constant arrays:
```cpp
// Constant static arrays
static const sc_int<5> g1[4] = {1,2,3,4};                
static const sc_int<5> g2[2][2] = {{1,2},{3,4}};        

SC_MODULE(MyModule) {
    static constexpr unsigned a[4] = {2,3,4,5};         
    static constexpr unsigned b[2][2] = {{2,3},{4,5}};  
    const unsigned c[4] = {2,3,4,5};  		        
    const unsigned d[2][2] = {{2,3},{4,5}};  		
}
```

1. Module/modular interface/record static and non-static functions supported. Global/namespace functions supported. Recursive functions not supported. 

1. Module/modular interface can contains thread and method processes. There are thread processes created with ```SC_THREAD``` and ```SC_CTHREAD``` to implement sequential logic. ```SC_THREAD``` must be sensitive to clock edge and optionally to reset(s). ```SC_CTHREAD``` must have clock edge as its second parameter. 

1. A thread process supports multiple asynchronous and synchronous resets as well as no reset. ```async_reset_signal_is``` and ```reset_signal_is``` can be used with any combination and any active level (1 or 0) .

1. There is method process created with ```SC_METHOD``` to implement combinational logic. ```SC_METHOD``` can be sensitive to reset signal, but not to clock (In the future, this feature will be supported, but now ```SC_METHOD``` cannot be used for sequential logic).

1. ```SC_METHOD``` must have all the signals/ports which are read in its body in the  sensitivity list. Method process without sensitivity list (empty sensitivity) is a special case. Such a method typically contains signal/port initialization with constant value. 
In ```SC_METHOD``` without sensitivity list local variable can be declared and used to store intermediate results. Module variable initialization can be done in such method process, but not recommended as can lead to concurrent assignment to the variable. Ternary operator (?) with arbitrary condition could be used. ```if``` statement with statically evaluated condition could be used. Read port/signal in such method is supported, but not recommended as can lead to different behavior in SystemVerilog vs SystemC (in SystemC process not activated if signal is changed). Loops and other control flow statements cannot be used here. In the SV one or more assign statements are generated for such method.
1. ```SC_METHOD``` cannot read signal/port which is modified in this process. Violation of this rule leads to incorrect ```always_comb``` in generated SV. 
1. C++ types ```bool```, ```char```, ```short```, ```int```, ```long int``` and ```long long int``` and their unsigned versions supported. SystemC data types ```sc_int```, ```sc_uint```, ```sc_bigint```, ```sc_biguint``` supported. 
1. Reference/constant reference types supported. Reference can be initialized with explicit object of the corresponding type, no conditional initialization supported. Constant reference can be initialized with variable, literal or constant expression. There are limitations on such constant expression: no array access, no function call, no ternary operator.
1. C++ types can be used together with SC types. Signed and unsigned types should never be mixed, as that can lead to unexpected result for operation with negative values. It is prohibited to assign negative value for unsigned constant or variable.
For operation with signed and unsigned arguments of ```sc_int```/```sc_uint``` types SystemC simulation differs from generated SV simulation. The tool constant evaluation complies to SystemC semantic, so it differs from SV simulation results. 

1. Result of operation in generated SV is the same as in correspondent operation in SC code. That means simulation results are equivalent. Only one exception is bitwise not ```~``` for C++ boolean type. As C++ boolean in promoted to ```int```, in bitwise operator all bits are considered that leads to different semantic than in generated SV. Therefore bitwise not ```~``` is prohibited for boolean argument, use logical not ```!``` instead. C++ boolean in promoted to ```int``` in bitwise operator, so generated SV differs from its source:
```cpp
bool a = true;
bool b = ~a;    // b == true
assert (b);
```
1. There are several operations where SystemC and SV have different rules for left operand cast: ```>>```, ```/```, ```%```. These operations cannot have as left operand expression with ```sc_bigint```/```sc_biguint``` variables or signals. Single ```sc_bigint```/```sc_biguint``` variable or signal/port object can be used there.Example for ```>>``` (the same true for ```\``` and ```%```): 
```cpp
sc_uint<32> a; 
sc_biguint<128> b; 
auto c = (a + 1) >> 8;  // OK: type of variable a is sc_uint
auto d = (b + 1) >> 8;  // Error: type of variable b is sc_biguint
auto e = b >> 8;        // OK: single sc_biguint variable used 
```
1. Operations with SC and C++ data type arguments can lead to overflow. Using such expressions as part of another expression can lead to non-equivalent simulation results. It is recommended to explicitly cast such expressions to the desired width. 
Left shift for C++ data types is cyclic which is not supported in the generated SV code. Therefore left shift overflow for C++ data type is prohibited. Example for expression with overflow:
```cpp
sc_uint<7> k, m;
k = 41; m = 42;
sc_uint<8> res = (k - m) % 11;   // Non-equivalent
res = sc_uint<8>(k - m) % 11;    // OK
unsigned u = 1;
res = u << 32;                   // Non-equivalent: 1 in SC, 0 in SV
```

1. Pointers are supported. Pointers are normally initialized at elaboration phase and de-referenced in process body. Pointer dereference ```*``` and ```->``` supported.  Pointer comparison supported. Obtaining object address with ```&``` is not supported. Operator ```new```/```new[]``` and operator ```delete```/```delete[]``` not supported. 
1. Local pointers to non-port/non-channel object supported. Local pointer to port/signal not supported. Pointer assignment is supported at local pointer variable declaration only. Local pointer without declaration allowed, but makes no sense. 
1. Pointer arithmetic not supported and general pointer assignment not supported. Pointer can be assigned to boolean variable, as well as, used in comparisons and conditions.
Function call with side effect is not supported in complex conditions with ```||``` and ```&&``` as function body is inlined and returned value stored in temporary variable.
1. Function can have parameters and returned value. Function can have local variable of non-signal/non-port type. Local variables can be non-static or constant static. No static non-constant local variables allowed. 
1. Function parameters can be passed by value, by reference, and by pointer, including pointer to channel. Constant reference parameter argument can be literal of the corresponding type.
Function can return result by value only. Return by reference or by  pointer not supported. Function with return type ```void``` can use ```return``` statement without argument. 
1. Record can be passed to function by value as well as by reference/constant reference. Record can be returned from function by value only. Function cannot return record by non-constant or constant reference. Module or modular interface cannot be passed to or returned from function.
1. Function with multiple ```return``` supported. Function ```return``` statement(s) is replaced with function result to variable assignment in SV code.  That leads to a function must have no code after ```return```. In particular, ```return``` statement in loop is not supported.  Function with return type void can have ```return``` statement(s) without argument. For ```return``` statement without argument no code is generated.
1. Module member function can access this module fields and call functions. 
1. Module member function can access a modular interface instance members. The accessed modular interface must be either child of the module or both of them must be modular interfaces instantiated in the same parent module. The accessed modular interface is flatten in its parent module.
Access to modular interface members allowed through a port interface (```sc_port<IF>```) or through a pointer to the modular interface. Pointer to modular interface supported in its parent module. Pointer to modular interface from another modular interface not supported.
1. Virtual functions supported. Function overload and hide function in child class supported.  
1. Conditions of ```if```, ```?```, ```for```, ```while```, ```do...while``` should be expression without side effects. Complex conditions with ```||```, ```&&```, ```!``` and brackets supported. If left part of logical expressions with ```||``` and ```&&``` evaluated as constant, right part code is not generated. That allows to check pointer is not null and do the pointer de-reference it in the condition expression. ```if``` and ```?``` conditions, including complex conditions, can contain function call without side effects and without ```wait()```. ```for```, ```while```, ```do...while``` conditions cannot have any function call.
Pointer assignment in general case can lead to different pointer values in branches/iterations possible. Pointer is replaced in generated code by its value variable, so it should have the same value at any Phi function. As this constraint is difficult to check, general assignment is prohibited.

1.  ```switch``` statement can have one or more cases including optional ```default``` case. Each case must have one and only one final ```break``` as the last statement of the case. ```default``` case also must contain final ```break```. Another option is empty case or empty default case. For empty case the next non-empty case (possibly default case) code is copied in the generated SV. 
1. ```switch``` case code can contain ```if```/loop statements as well as inner ```switch``` statements. ```switch``` case code can contain function calls. ```switch``` statement in called function can contains ```return``` statements in the end of all cases. For such ```switch``` cases final ```break``` statement not allowed, no mix of ```return``` and ```break``` supported.
1. ```for``` loop can have only one counter variable with optional initialization, condition and increment sections. The variable can be declared in the loop initialization section or before the loop. 
     *  Initialization section can have simple variable initialization or assignment only, cannot have function call. 
     *  Condition section can have one comparison operator for the loop variable, cannot have function call. 
     *  Increment section can have increment or decrement of the loop variable, cannot have function call.
1. An empty loop is removed by default. There is no issue with loop condition as it cannot contain any side effects. There can be issue with the removed loop counter,  if it declared outside and used after the loop. To avoid empty loops/```if```/```switch``` removing ```NO_REMOVE_EXTRA_CODE``` option should be used.
1. Arrays are supported as module members and function local variables. Multidimensional arrays supported. Array of variables, array of constants and  array of static constants supported. Array of non-constant pointers supported. Array of constant pointers not supported yet. Array of signals/ports supported. Array of signal/port pointers supported.
1. Array in-place initialization supported. Array initialization by zeros with empty brackets ```{}``` supported. 
Example for array initialization:
```cpp
int a[3][2] = {};                 // Initialization by zeros
sc_uint<4> b[3][2];               // Initialization by zeros for SC integer, the same as using {}
int c[3][2] = {1,2};              // Partial initialization
int d[3][2] = {1,2,3,4,5,6};      // All elements initialization
int e[3][2] = {{1,2,3},{4,5,6}};  // All elements initialization
```
1. Array of modules supported. Array of pointers to module/modular interface supported. In array of module/modular interface pointers all elements must be the same class (virtual functions analyzed for the first element only). Array of modular interfaces not supported, use ```sc_vector``` instead. Array of port interfaces (```sc_port<IF>```) not supported. 
1. Array of records supported. Record which is element of an array must have default constructor and no in-place member initialization. Array of records cannot be initialized with initializer list, therefore array of constant records does not make sense. Array of pointers to record is not supported.
1. Array of any pointers, including modular interface pointers, must be homogeneous: 
    * all elements must be ```nullptr``` or created with operator ```new```, no pointers to existing objects allowed,
    * all modular interface/record, pointed by array elements, must have the same process code and member function code. Example for non-homogeneous array of modular interfaces:
```cpp
struct A : sc_module, sc_interface {
   bool par;
   A(bool par_) : par(par_) {}
   int f() {
       return (par ? 1 : 2);     // Different code depends on ctor parameter
   } 
};
struct B : sc_module {
   A* a[N];
   B () {
      for (int i = 0; i < N; ++i) 
          a = new A(i==0);       // Different modular interface -- not supported 
   } 
};

```
1. Record/modular interface in array cannot have members of array of pointers, array of port interfaces, and array of record/modular interface. Such record/modular interface can have non-pointer member and arrays of data and ports/signals.
1. Array of record cannot be accessed at index which is the same array element, for example: ```a[a[i].m].m;``` -– not supported, ```a[b[i].n].m``` –- supported.
1. Array and array of pointers, including array of signals/ports, can be passed into function. Multidimensional array also 
can be passed into function.

1. ```std::array``` supported for SC objects (modules, signals, ports) as well as for data types. For data types ```std::array``` could be non-constant, constant or static constant (```static constexpr```). ```std::array``` can be  module member as well as function local variable. ```std::array``` can contain elements of ```std::array``` type, i.e. multi-dimensional arrays supported. 

1. ```std::pair``` of data types module member supported. ```std::pair``` could be constant or non-constant, no function local ```std::pair``` supported.
1. SC vector (```sc_vector```) supported for signals, ports, modules and modular interfaces. Multi-dimensional vector (vector of vectors) also supported. SC vector instances can be instantiated in modules and modular interfaces, including array of modular interfaces. SC vector cannot be passed to or returned from function.
1. Uninitialized local variable of SystemC data type (```sc_uint```, ```sc_biguint```, ...) has got zero value in generated code as it got in the default constructor.
1. Non-constant module fields cannot be initialized in constructor or in place. After elaboration phase non-constant fields have unknown value. They need to be initialized in reset section of a process. 

1. Module member constants can be initialized in place, in module constructor initialization list and in ```before_end_of_elaboration``` callback. Such constants are translated into ```localparam``` in SV. 
Example of constant initialization in ```before_end_of_elaboration```
```cpp
const unsigned* C = nullptr;
void before_end_of_elaboration() override {
   C = sc_new<unsigned>(42);
}
```
Member constants of any integral type should be 64 bit or less (stored in uint64/int64 field). Such constants with more than 64 bit has unknown value after elaboration, therefore error is reported.
Local constants in process function are translated to SV local variables. Such constants can be more than 64bit.

1. Explicit type cast in C style ```((T)x)```, functional style ```(T(x))```, and static cast ```static_cast<T>(s)``` supported for right side of assignment statement and function arguments. Type cast for left side of assignment is ignored. Constant cast ```const_cast``` is prohibited in left part of assignment, and ignored elsewhere. Reinterpret and dynamic type casts are not supported. Type cast to base class supported for function call ```(T::f())``` and member access ```(T::m)```.

1. Explicit type cast can be used to change width or/and signness of the variable, literal or expression. Multiple casts for one object are supported. Type cast to change unsigned object to signed is supported in binary, unary and compound operations. In other operations type cast to signed as well as all type casts to unsigned are ignored. That prohibits to cast negative value to unsigned and cast unsigned with set high bit to signed negative. 

1. Dead code is eliminated from analysis and SV generation. Dead code is determined by evaluating condition based on compilation time constants (static constant and constexpr supported).  Dead code determination does not use loop counters and other variables even they can be evaluated as constant.  Accessing not allocated object and arrays in dead code and only in dead code allowed. Accessing not allocated objects under non dead code or dead code which cannot be evaluated in described manner is prohibited and leads to synthesis error.


## Supported C++ and SystemC types

It is recommended to use SC data types instead of C++ types where it is possible. Using C++ data types make sense for 1bit value -- ```bool``` type and as ```for``` -- loop iterators.
Using ```sc_bigint<N>```/```sc_biguint<N>``` results in more accurate arithmetic than ```sct_int<N>```/```sct_uint<N>``` and is free from implicit promotion to ```int64_t```/```uint64_t```. Drawback of ```sc_bigint<N>```/```sc_biguint<N>``` is simulation speed slow down.

Supported types conversion into SV types are given in the table below.
 
| C++/SC type               | SV type           | SC synthesizable standard |
| ------------------------- | ----------------- | ------------------------- |
| ```sct_uint<N>```         | logic signed [N]  | nothing if N = 0  |
| ```sct_int<N>```          | logic signed [N]  | nothing if N = 0  |
| ```sc_int<N>```, ```sc_bigint<N>```   | logic signed [N]  |  |
| ```sc_uint<N>```, ```sc_biguint<N>``` | logic [N]	        |  |
| ```sc_bv<N>```                        | logic [N]         |  |
| bool	                    | logic             |  |
| char, signed char, int8_t | logic signed [8]  | ```const char*``` considered as string literal |
| unsigned char, uint8_t    | logic [8]         |  |
| short, int16_t            | logic signed [16] |  |
| unsigned short, uint16_t  | logic [16]        |  |
| int, int32_t              | integer           | 32bit |
| unsigned int, uint32_t    | integer unsigned  | 32bit |
| long                      | logic signed [64] | 32/64bit depends on platform |
| long long, int64_t        | logic signed [64] | 64bit |
| unsigned long             | logic [64]        | 32/64bit depends on platform |
| unsigned long long, uint64_t | logic [64]     | 64bit |
| __uint128_t               | logic [128 ]      |   |
| __int128_t                | logic signed [128] |  |

As long and unsigned long always generated as logic 64bit, only 64bit platforms are supported.

### Literals
Literals up to 4096 symbols in given radix are supported. Literals with more than 64bit can be provided as ```string``` or ```const char*``` literal or constant variable. 

### Unsupported C++ and SystemC types
* sc_event
* sc_fifo, sc_mutex, sc_semaphore, sc_buffer
* sc_signal_resolved, sc_signal_rv
* sc_inout
* sc_logic , sc_lv, sc_bit
* fixed-point types: sc_signed, sc_unsigned, sc_fix, sc_ufix, sc_fixed, sc_ufixed 
* floating-point types: float, double
* sc_export


### Mixing signed and unsigned types

Never mix signed and unsigned types as arguments of one operations. There are different rules to promote C++/SystemC types to signed or unsigned, which can give unexpected result. Unsigned types should be used for non-negative values only, and in expression which always evaluated as non-negative.

