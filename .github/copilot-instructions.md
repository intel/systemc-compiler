Repository: Intel ICSC (SystemC compiler) designs and examples. 
Version 1.6

Short guidance for GitHub Copilot / AI assistants working on this repository.

1) Purpose
- This repo contains the Intel SystemC Compiler toolchain, SystemC library extensions (in `components/common/sctcommon` folder), SystemC designs (in `designs` folder), examples (in `examples` and `tests` folders).
- Focus on synthesizable cycle-accurate SystemC code with high level channels, SystemC assertions and utility functions declared in headers in `components/common/sctcommon` folder.

2) Build & run examples
- Use the project's normal CMake build flow. 
- Use `svc_target` cmake function with `ELAB_TOP` parameter specifies design module instance name, for example `svc_target(copilot_chan_demo ELAB_TOP tb_instance.dut_instance)`

3) SystemC main objects
- Use synthesizable SystemC language for design code and arbitrary SystemC language for testbench code.
- Module is a C++ class or a structure inherits `sc_module`. 
- Modular interface (MIF) is a module which inherits `sc_interface`. Modular interface is a special module which functions can be called from its parent module or another modular interface which is a child module of the same parent module.
- Any module and modular interface must have at least one constructor. Module and modular interface constructor must have `sc_module_name` parameter. Module and modular interface constructor can contain arbitrary C++ code.
- There is one top module in a SystemC design. Top module can contain module instances as well as other fields. All instances are module fields, no global/namespace variables supported. Global/namespace constants supported.
- Module, modular interface and record non-static data members and static constant data members supported. Constant static arrays are supported: static constexpr member array with in-place initialization; const non-static member and global array with in-place initialization.
- Module/modular interface/record static and non-static functions can be used. Try to avoid global/namespace functions. Do not use recursive functions.
- Signal is a SystemC `sc_signal`. 
- Port is a SystemC `sc_in` and `sc_out`.
- Channels are `sct_target`, `sct_initiator`, `sct_fifo`, `sct_pipe`, `sct_buffer`, `sct_register` and other components declared in `components/common/sctcommon/sct_ipc_if.h`.

4) Data types 
- Use `sct_int` and `sct_uint` types from `components/common/sctcommon/sct_sel_type.h` instead of SystemC integer data types. `sct_int` and `sct_uint` types can have arbitrary width, including zero bit. Zero width variables can be used for optional variables or input/output ports. 
- Use C++ `bool` type for one bit variables if they intended for control, like `ready`, `request` or `enable`. Use `sct_uint<1>` type for one bit data variables.
- Use C++ `unsigned` type for `for` loop iterator except case when iteration number exceeds 2^32.
- C++ types can be used together with SC types. Do not mix signed and unsigned types. Do not assign negative value to unsigned constant or variable.
- Do not use bitwise not `~` with type `bool` argument, use logical not `!` instead.
- Do not use right shift (`>>`), division (`/`) and modulo operator (`%`) with the left operand of `sc_bigint`, `sc_biguint` type.
- Try to avoid overflow in subexpression as part of another expression. Use explicit cast for such sub-expressions to the desired width.
- Try to avoid using any pointers and dynamic memory allocation. Any pointer can be initialized in module constructor only. Do not use any pointer arithmetic except comparison with `nullptr`.
- Use type cast to change expression/variable width or signness. Type cast in C style `(T)x`, functional style `T(x)`, and static cast `static_cast<T>(s)` can be used. Do not use constant cast `const_cast`, reinterpret cast and dynamic cast. Type cast to base class supported for function call `T::f()` and member access `T::m`.
- Arrays can be used as module members and function local variables. 
- Array in-place initialization supported. Array initialization by zeros with empty brackets `{}` can be used.
- Use `sc_vector` from SystemC to create a collection of signals, ports, channels or modules. 
- Use C array or `std::array` to create a collection of other objects. std::array can contain elements of `std::array` type, i.e. multi-dimensional arrays supported.
- Pair of data `std::pair` type is supported.
- Use `SC_NAMED()` macro to instantiate module, channel or `sc_vector` and provide its name, for example: `sc_signal<bool> SC_NAMED(sig)` instead of `sc_signal<bool> sig("sig")`.
 
5) Method and thread processes 
- Use `SC_METHOD` macro to create a combinational logic.
- Use `SC_THREAD` macro to create a sequential logic. 
- Every `SC_METHOD` and `SC_THREAD` in design code must have an explicit sensitivity list. A process sensitivity list must contain all signals, ports, channels whose values are read or used inside the process body. Do NOT add any reset signal/port to a process sensitivity list.
- To specify signals or output ports assigned to constant values use a `SC_METHOD` process without sensitivity list.
- Do not read or use signal, port, channels in `SC_METHOD` process if they are modified in this process. 
- Do not check or use reset port/signal in a `SC_METHOD` process.
- Never use `dont_initialize()` in design code.
- Every `SC_THREAD` process must have one asynchronous reset with low active level specified with `async_reset_signal_is(reset, false)`.
- Each `SC_THREAD` process function should have reset section with declarations of local variable and initialization of output ports, signals and channels. The reset section should finish with `wait` call. 
- Each `SC_THREAD` process function should have main behavioral loop. Prefer simple implementation of the main behavioral loop with one `wait` call at the end.
- Avoid to use `break` and `continue` in the main behavioral loop as well as other 
loops with `wait` calls. 
- Avoid holding references to `sc_signal` read results. Use local variables for safety.
- Initialize module member variables in a process code, not in-place nor in module constructor.
Initialize module member constants in place or in module constructor initialization list.
- Use functions if that makes sense. Function can have parameters and returned value. Function can have local variables which are non-static or constant static. Do not use static non-constant local variables.
- Use references and constant references for parameters passed to a function. Do not use reference to return a result from a function, return result by value only. 
- `return` should be the last statement in a function if any. Do not use multiple `return` in a function.
- Virtual functions can be used. Overloaded function can be used.
- Never use side effects in conditions of `if`, conditional operator (`?`), `for`, `while`, `do...while` loops. Complex conditions with `||`, `&&`, `!` and brackets can be used. 
- Try to avoid function calls in `if` and `?`  conditions except signals, ports, channels methods. Do not use function calls in `for`, `while`, `do...while` conditions.
- `switch` statement can have one or more cases including optional `default` case. Each `case` must have one and only one final break as the last statement of the case. `default` case also must contain final break. Another option is empty case or empty `default` case. 
- `switch` case code can contain `if` and loop statements as well as inner `switch` statements. `switch` case code can contain function calls. 
- Use only one iterator in `for` loop. Use only simple condition in `for` loop with its iterator, for example for iterator `i` and number of iteration `N` condition can be: `i < N` or `i != N`. Use only simple increment of `for` loop iterator for example for iterator `i` increment can be: `++i`.

6) Records
- Record is a C++ class or struct which does not inherit `sc_module` in any way. 
- Use record to combine multiple data together. Use record for ports, signals and channels as well as local and member variables.
- Record can have member functions and data members. Record cannot have another record members. Record cannot have any pointer or reference members. 
- Record can have constructor, field in-place initialization and initializer list.  Record cannot have non-default copy/move constructors and operators. Record can have user defined `operator ==()` which should have default semantic.
- Record can have one or multiple base classes. Record base class cannot have constructor body (constructor body should be empty), but can have initialization list and in-place initialization. Virtual functions in records are not supported.
- Record can be used as type of signal, port, channel.
- Record can be passed to function by value as well as by reference. Record can be returned from function by value only. 
- Array of records supported, such a record must have default constructor and no in-place member initialization. 

7) Channels
- There are the following channels: `sct_target`, `sct_initiator`, `sct_fifo`, `sct_pipe`, `sct_buffer`, `sct_register` and other components declared in `sct_ipc_if.h`. 
- Use `sct_target` and `sct_initiator` pair where it needs to have handshake between two modules. 
- Use `sct_fifo` for interprocess communication inside of a module. Use `sct_fifo` to store multiple chunks of data in a process.
 `sct_fifo` capacity is specified by template parameter `LENGTH`.
- Use `sct_buffer` as `sct_fifo` if it is accessed from `SC_THREAD` process or two `SC_THREAD` processes.
- Use `sct_in` instead of `sc_in`, use `sct_out` instead of `sc_out`.
- Use `sct_signal` instead of `sc_signal`.
- Use `sct_clock` instead of `sc_clock`.
- Use `sct_pipe` to retime logic into multiple cycles.
- Use `sct_register` to keep state for `SC_METHOD` process.
- Use `sct_ff_synchronizer` as 2 or 3 flip-flop synchronizer.

- Channels common methods are:
  - `reset_put()` to reset for put, 
  - `reset_get()` to reset for get,
  - `ready()` to check if the channel is ready to put,
  - `put()` put a request if thechannel  is ready,
  - `request()` to check if there is a request,
  - `peek()` to peek request data, but not get the request,
  - `get()` to get a request if there is any,
  - `b_get()` to get a request if there is any or wait for a request,
  - `b_put()` to put a request if the channel is ready or wait when the channel,
  - `bind()` to bind initiator to target or target to initiator, 
  - `operator <<()` to add the channel to a process sensitivtity,
  - `clk_nrst(clk, nrst)` to connect clock and reset.
 
 - `sct_fifo` additional methods:
  - `size()` to get the number of fifo slots which is `LENGTH`,
  - `almost_full(N)` to check is the fifo has N or less empty slots,
  - `almost_empty(N)` to check is the fifo has N or less elements,
  - `elem_num()` to get number of elements updated in the last cycle. 

- Do not access one varible in more than one process. Do not write to a channel or a signal in more than one process.
- Reset channels in a `SC_THREAD` process reset section and in the beginning of a `SC_METHOD` process. Use `reset_put()` to reset `sct_initiator` and `reset_get()` to reset `sct_target`. Use `reset_put()` for `sct_fifo` in a process which puts to the fifo. Use `reset_get()` for `sct_fifo` in a process which gets from the fifo. 
- Add `sct_target` to a process which gets from it, for example `sensitive << target`. Add `sct_initiator` to a process which puts to it, for example `sensitive << initiator`. Add `sct_pipe` to a process where it is used, for example `sensitive << pipe`.
- Add `sct_fifo` to a process where put is perfromed with `PUT` suffix, for example `sensitive << fifo.PUT`. Attach a fifo to a process where get is perfromed with `GET` suffix, for example `sensitive << fifo.GET`.
- Methods `elem_num()`, `ready()` and `request()` return a result of the last cycle, which is not updated after put or get elements in the current cycle.
- Use `clk_nrst(clk, nrst)` to connect clock/reset to channels when available.
- Any port should be bound to a signal. Any `sct_target` should be bound to a `sct_initiator` with method `bind()`.
- To interconnect a child module to its parent module `sc_port<sct_target>` or `sc_port<sct_initiator>`. In this case a channel in the child module is promoted to the parent module thourg the channel port to be bound.
- To interconnect MIF child module to its parent module `sct_fifo` in the child MIF and `sc_port<sct_fifo>` in the parent could be used.
- Do not update references to the result of `sct_signal::read()`; read into a local copy, modify, then `write()` back.

8) Add the following files into context:
- `components/common/sctcommon/sct_ipc_if.h` for channels and other components declarations. 
- `components/common/sctcommon/sct_target.h` for `sct_target` definition.
- `components/common/sctcommon/sct_initiator.h` for `sct_initiator` definition.
- `components/common/sctcommon/sct_fifo.h` for `sct_fifo` definition.
- `components/common/sctcommon/sct_pipe.h` for `sct_pipe` definition.
- `components/common/sctcommon/sct_clock.h` for `sct_clock` definition.

9) Code style & patterns
- Generate code with no more than 80 symbols per line.
- Keep SystemC modules small and deterministic.



