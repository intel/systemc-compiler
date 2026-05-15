---
name: systemc-tools
description: "Use when writing, editing, reviewing, or debugging synthesizable SystemC code for Intel SystemC Compiler (ICSC) and SingleSource library. Covers rules for module hierarchy, channels and ports, process declarations, reset behavior, sensitivity lists, SystemC and C++ data types and collections. Applies communication channels, memory modules, data types and utility functions from Single Source library."
argument-hint: "Describe the SystemC coding or review task and the files involved"
---

# SystemC Tools

Use this skill for synthesizable SystemC code development and review in projects using ICSC and the SingleSource library. Apply it when implementing modules, records, channels, process code, assertions, or when reviewing existing code for synthesis, simulation, and SystemVerilog generation risks.

## Workflow

1. Identify the task type: implementation, refactor, debug, or review.
2. Read the nearby code first: module ports, constructor bindings, process declarations, sensitivity lists, reset logic, channel usage, and tests.
3. Check synthesis constraints before changing behavior: process kind, supported data types, control flow, function calls, dynamic allocation, and pointer usage.
4. Prefer existing project patterns over introducing new abstractions.
5. For edits, keep changes local to the relevant module, record, or helper.
6. Validate with the narrowest relevant test or build target. For CMake projects, use the CMake Tools build/test tools.
7. In reviews, lead with correctness, synthesis mismatch, reset/sensitivity, handshake, and missing-test risks.

## Project Structure

- Expect a main source file such as `sc_main.cpp` containing the SystemC entry point and testbench/top instantiation.
- A design normally has one translated top module, selected by `ELAB_TOP` when required.
- Keep design modules in headers/source files organized by protocol or block ownership.
- Keep testbench-only logic separate from synthesizable module logic.
- Avoid global or namespace variables for design state. Global and namespace constants are acceptable when they are compile-time constants.

## Module Hierarchy

- A module is a C++ class or structure inheriting `sc_module`.
- A modular interface inherits both `sc_interface` and `sc_module`; ICSC flattens it into the parent module in generated SystemVerilog.
- A pure interface inherits `sc_interface` only and is not part of the module hierarchy.
- Instantiate modules and modular interfaces inside parent modules, except top-level instances in `sc_main` or testbench hierarchy.
- Do not pass modules, modular interfaces, or pointers to them as function parameters or return values.
- Do not use module or modular interface types as signal or port payload types.

## Modules And Modular Interfaces

- Use explicit ports, signals, and SingleSource channels for interconnect.
- Avoid direct field access across normal module boundaries.
- Access child modular interface fields and methods only when they are flattened into the same generated module.
- Access modular interface methods through `sc_port<IF>` or a parent-owned pointer only when the connected interface is valid for flattening.
- Keep module ownership clear: one parent owns each child module/interface instance.
- Avoid multiple pointers to the same heap-allocated SystemC object.

## Module Constructors

- Every module and modular interface needs a constructor with `sc_module_name`, or `SC_CTOR` when suitable.
- Constructor bodies may create processes, bind ports/channels, allocate elaboration-time objects, and call elaboration-time helper functions.
- Prefer static/member object construction over dynamic allocation.
- If dynamic allocation is needed, use it only during elaboration. Use `new` for SystemC objects where supported, and `sc_new` or `sc_new_array` for non-`sc_object` data.
- Bind all child ports and channels in constructors. Unconnected child ports may be promoted to top-level ports.
- Initialize constants in-place, in constructor initializer lists, or in supported elaboration callbacks.
- Initialize non-constant runtime state in reset code, not as a substitute for reset behavior.

## Process Code

- Use `SCT_METHOD` to create combinational method process. Use `SCT_THREAD` macro to create sequential thread process. Use `SC_CTHREAD` macro only in testbench and only if it required, for example for process without sensitivity. Never use `SCT_CTHREAD` macro.
- Use method processes for combinational logic and simple sequential methods where project conventions allow.
- Use thread processes when multiple states or explicit `wait()` calls simplify the design.
- A combinational method must be sensitive to every signal, port, or channel it reads.
- A method must not read a signal or port that it writes in the same process.
- A thread reset section is the code before the first `wait()`; reset modified channels and register variables there.
- Avoid to have any code between reset section `wait()` and the main loop. One exception is if multi-cycle initialization is required for an object like memory or complex data structure used in the process.
- Keep the main thread loop simple. Prefer one `wait()` at the end of the loop unless a real multi-state protocol is needed.
- Assertions in thread processes should be before the main loop, not inside it, unless using a supported loop assertion form.
- Do not put unsupported function calls or side effects in process sensitivity, loop conditions, or complex conditions.

## Data Types And Literals

- Prefer SystemC/SCT integer types over C++ integer types for hardware-width values.
- Use `bool` for one-bit Boolean intent and loop variables such as `int` or `unsigned` for simple static loops.
- Prefer `sct_uint<N>` and `sct_int<N>` for bit-accurate widths, including optional zero-width fields.
- Use `sc_biguint<N>` and `sc_bigint<N>` when exact wide arithmetic behavior is more important than simulation speed.
- Avoid unsupported types: `sc_lv`, `sc_logic`, `sc_signed`, `sc_unsigned`, fixed-point types, and floating-point types in synthesizable code.
- Do not mix signed and unsigned values in one expression unless explicitly cast and reviewed.
- Use string literals for numeric literals wider than 64 bits.
- Use `sct_zeros<N>` and `sct_ones<N>` for all-zero and all-one SCT literals.
- Remember that uninitialized local SystemC integer variables are zero-initialized by their constructors.

## Channels And Ports

- Use `sct_signal`, `sct_in`, and `sct_out` for SingleSource signal and port interconnect when that is the project convention.
- Use `sc_signal`, `sc_in`, and `sc_out` where the surrounding code already uses standard SystemC ports/signals.
- Connect sibling modules through parent-owned signals or through supported SingleSource channels.
- Bind ports through hierarchy only when ownership and direction remain clear.
- Ensure all ports are connected in design or testbench; otherwise expect top-port promotion.
- For target/initiator channels, bind endpoints in the common parent and connect clock/reset with `clk_nrst()` where required.
- Add every accessed SingleSource channel or signal/port to process sensitivity according to the process kind.
- Avoid combinational loops in FIFO/channel ready-valid paths.

## Collections

- Built-in arrays and `std::array` are supported for data and many SystemC objects, but follow local code style.
- Prefer `sc_vector` for arrays of signals, ports, modules, and modular interfaces when supported.
- Keep pointer arrays homogeneous. Elements should be all `nullptr` or all allocated objects of the same concrete class.
- Avoid non-rectangular pointer data structures except for supported module-array cases.
- Do not index a signal/port array or record array using the same array element when that pattern is unsupported.
- Do not pass `sc_vector` to or return it from functions.

## Structures

- Use records for plain data payloads, not for modules, channels, signals, ports, or ownership.
- Records used in channels or ports should have a default constructor, `operator==`, `operator<<`, and `sc_trace` when required by the channel/port type.
- Keep records trivially copyable where they are passed by value or returned from functions.
- Records cannot contain pointers, references, modules, modular interfaces, signals, or ports in synthesizable payload code.
- Avoid nested records when ICSC restrictions apply.
- Record constructors may contain function calls, but field in-place initialization and initializer lists must not call functions.

## Control Flow

- Conditions for `if`, `?:`, `for`, `while`, and `do while` must be side-effect free.
- `if` and ternary conditions may call side-effect-free functions without `wait()`.
- `for`, `while`, and `do while` conditions must not call functions.
- Method `range()` arguments for  `sct_uint`, `sct_int` and other bit-accurate types in loop if loop counter is used as index should use of the following form: `range(i+N, i)` or `range(i, i-N)`or `range(M*i + N, M*i)` where `i` is the loop counter, `M` and `N` are constants. The same is applicable for `operator ()` for  `sct_uint`, `sct_int` and other bit-accurate types which has the same semanthic as `range()`.
- Static loops without `wait()` must have statically determinable iteration counts.
- Loops with `wait()` may have dynamic iteration counts, but every path must respect process wait/reset rules.
- `switch` cases need one final `break`, or return-only cases in supported function patterns. Do not mix `break` and `return` styles in one switch.
- Avoid `break` and `continue` in main loops and loops containing `wait()`.
- Do not use `goto`.

## Arithmetical Operations

- Operator comma (`operator ,`) for `sct_uint`, `sct_int` and other bit-accurate types concatenates arguments therefore it requires correct order of arguments and their bit widths.
- Preserve bit widths intentionally. Cast intermediate expressions when overflow or promotion could change generated SV behavior.
- Never mix signed and unsigned operands casually; this is a common SystemC versus SV mismatch source.
- Avoid bitwise `~` on C++ `bool`; use logical `!`.
- Review `>>`, `/`, and `%` carefully with `sc_bigint` or `sc_biguint`; use a single big variable operand where required.
- Avoid left-shift overflow on C++ integer types.
- For protocol bit packing, keep struct member order, documented bit ranges, `toBits()`, and `fromBits()` in exact agreement.
- Prefer explicit width constants and typedefs for packed fields.

## Functions

- Functions may be module, modular interface, record, namespace, or static/non-static member functions when supported.
- Recursive functions are not supported.
- Local variables in functions must be non-static or constant static. Do not use static non-constant locals.
- Pass parameters by value, reference, const reference, or supported pointer forms only.
- Return by value only. Do not return references or pointers.
- Functions with multiple returns are allowed only when no code follows a return path.
- Do not return from inside loops.
- Do not pass modules or modular interfaces to functions.
- Keep helper functions side-effect-free when used in conditions or assertions.

## SingleSource Library

- Include `sct_common.h` through the project’s established include path when using SingleSource channels and utilities.
- Use `sct_target` and `sct_initiator` for 1:1 request/response connections.
- Use multi-target or multi-initiator channels only for explicit 1:N or N:1 structures.
- Use `sct_fifo` for buffering between processes; choose `sync_valid`, `sync_ready`, and depth based on producer/consumer process types.
- Use `sct_buffer` for fast sequential buffering where its restrictions match the design.
- Use `sct_pipe` for pipeline registers and retiming-friendly logic.
- Use `sct_register` for state in method processes.
- Reset channels in the reset section of the process that writes/modifies them.
- Keep TLM and RTL mode behavior equivalent unless the task explicitly concerns performance modeling.

## Assertions

- Prefer temporal assertions `SCT_ASSERT` in module scope or `SCT_ASSERT_THREAD` before the main thread loop.
- Use `sct_assert` for immediate checks only when temporal assertions do not fit.
- Include `sct_assert.h` when using ICSC assertions.
- Assertion expressions must not contain unsupported function calls or ternary operators.
- Use `SCT_ASSERT_LOOP` for assertions in supported static loops before the main thread loop.
- Use `sct_assert_latch` only when an intentional latch is required and reviewed.
- Keep assertion time parameters constant or statically evaluable.

## Types And Utility Functions

- Use `sct_uint<N>` and `sct_int<N>` for width-generic hardware fields.
- Use `sct_nbits<X>`, `sct_addrbits<N>`, and `sct_addrbits1<N>` for derived widths instead of hand-coded magic numbers.
- Use `sct_log2`, `sct_floor_log2`, `sct_ceil_log2`, `sct_is_pow2`, and `sct_next_pow2` for compile-time width/math utilities.
- Use `sct_min`, `sct_max`, `sct_popcount`, `sct_bin_to_1hot`, and `sct_1hot_to_bin` when they match the operation clearly.
- Keep utility use synthesizable and compatible with the target ICSC subset.

## Coding Style

- Match the surrounding project style for naming, indentation, process layout, and constructor binding order.
- Prefer explicit field-width constants and type aliases for protocol structures.
- Keep reset behavior obvious and close to the modified state.
- Keep process responsibilities narrow: one process should own writes to a signal/channel state object unless the channel explicitly supports otherwise.
- Avoid clever C++ constructs when straightforward SystemC maps more predictably to SV.
- Do not compare `bool` values to `true` or `false` explicitly; use them directly in conditions.
- Add brief and clear comments to clarify protocol intent like protocol states, hardware assumptions and other non-obvious design considerations.
- Keep comments focused on protocol intent, hardware assumptions, or non-obvious synthesis constraints.
- For reviews, check behavior first, then synthesis compatibility, then style.

## Naming

- Do not use names beginning with `sct_` or `SCT_` for user-defined entities.
- Avoid suffix `_next` for user variables because it may be reserved for generated register variables.
- Use names that reflect hardware intent: valid, ready, request, response, payload, state, index, mask, count.
- Keep generated SV name stability in mind when renaming ports, processes, or channel instances.
- Prefer instance names matching member names, using `SC_NAMED` where the project convention requires it.

## Logging And Others

- Use simulation logging only where it helps debug tests; keep synthesizable design paths free of non-synthesizable side effects.
- Provide `sc_trace` helpers for record/channel payload types when waveforms or ports require them. 
- Guard debug-only traces and logs with `SCT_DEBUG` macro when appropriate. 
- Enable `SCT_DEBUG` for the project CMake target for debugging and using `sc_trace`. Do not enable `SCT_DEBUG` for ICSC synthesis target specified with `svc_target()`.
- Do not use unsupported dynamic memory allocation in process code.
- Avoid unsupported types such as unions in synthesizable code.
- Use build and test results to confirm changes; do not rely only on code inspection for protocol or reset changes.

## Review Checklist

- Is every process sensitivity list complete and minimal enough for the process kind?
- Is reset behavior defined for every stateful field/channel modified by the process?
- Are handshakes free of combinational loops and lost requests?
- Are signedness, width, overflow, shifts, division, and modulo operations explicit enough?
- Are packed struct field order, comments, widths, and conversion functions consistent?
- Are arrays, records, pointers, and dynamic allocation within the supported subset?
- Are assertions placed in supported locations and written with supported expressions?
- Are tests or simulations covering the changed protocol behavior?

## Reference Material

When detail is needed, consult the workspace documentation:

- `doc/skills/CompilerUserGuide.md`
- `doc/skills/SystemcSupported.md`
- `doc/skills/SingleSource.md`
- `doc/skills/Assertions.md`
- `doc/skills/TypesUtility.md`


## Code Repositories

When creating or reviewing code, refer to the following repositories for examples of typical usage patterns and project structure:

| Repo | GitHub Path | Purpose |
|---|---|---|
| **SystemC common library** | `https://github.com/intel/systemc-compiler/tree/main/components/common/sctcommon` | SingleSource communication channels, memories, AMBA ports and utilities |
| **SystemC common library examples** | `https://github.com/intel/systemc-compiler/tree/main/components/common/test` | Unit level tests and usage examples for the SystemC common library |
| **Intel SystemC Compiler (ICSC) examples** | `https://github.com/intel/systemc-compiler/tree/main/designs/examples` | SystemC Compiler design examples |
| **Intel SystemC Compiler (ICSC) examples with SingleSource channels** | `https://github.com/intel/systemc-compiler/tree/main/designs/single_source` | SystemC Compiler previous designs |
| **Intel SystemC Compiler (ICSC) unit tests** | https://github.com/intel/systemc-compiler/tree/main/designs/tests | SystemC Compiler unit tests can be used as synthesizable code references |



