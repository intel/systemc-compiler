# Integer types ```sct_int``` and ```sct_uint```

The bit-accurate integer types which support any number of bits:
* ```sct_int<N>``` is signed integer with N bits, where N is zero or positive 
* ```sct_uint<N>``` is unsigned integer with N bits, where N is zero or positive 

SystemC contains ```sc_int```/```sc_uint``` integer types which are limited to 64 bit. Also SystemC contains ```sc_bigint```/```sc_biguint``` which can be more than 64bits, but are slow in simulation. Also all these integer types do not support 0 width.
```sct_int``` and ```sct_uint``` automatically substitute ```sc_int```/```sc_bigint```(```sc_uint```/```sc_biguint```) depends on bit width. These types are implemented in ```sct_sel_types.h```.

To initialize a variable of ```sct_uint``` type with all bits 0 or 1 there are special templates: 
* ```sct_zeros<N>``` is N-bit zeros literal of sct_uint<N> type,
* ```sct_ones<N>``` is N-bit ones literal of sct_uint<N> type.

```cpp
// Variable declaration with 0/1 initialization
sct_uint<12> a = sct_zeros<12>;
sct_uint<66> b = sct_ones<66>;
auto c  = sct_zeros<90>;
```

## Zero width integer types

Zero width integer types ```sct_int<0>``` and ```sct_uint<0>``` intended to represent optional variables, signals, ports and record fields. In the generated SV such variables are not declared, assignment to such variable is not generated, using such variable as RValue replaced with ```0```. The same works for zero width signals, ports and record fields.

```cpp
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

# Utility functions

* ```sct_min(a, b)``` returns (b < a) ? b : a
* ```sct_max(a, b)``` returns (a < b) ? b : a
* ```sct_popcount(val)``` returns number of bits set to 1, e.g. sct_popcount(0x85) == 3, sct_popcount(0x11) == 2, sct_popcount(0) == 0
* ```sct_bin_to_1hot(val)``` converts binary value to one-hot value, e.g. sct_bin_to_1hot(0) == 1, sct_bin_to_1hot(2) == 4, sct_bin_to_1hot(3) == 8
* ```sct_1hot_to_bin(val)``` converts one-hot value to binary value, e.g. sct_1hot_to_bin(1) == 0, sct_1hot_to_bin(4) == 2, sct_1hot_to_bin(8) == 3
* ```sct_log2<X> / sct_floor_log2<X>``` compile-time floor(log2(X)) value, e.g. sct_log2<1> == 0, sct_log2<2> == 1, sct_log2<3> == 1, sct_log2<4> == 2
* ```sct_ceil_log2<X>``` compile-time ceil(log2(X)) value, e.g. sct_ceil_log2<1> == 0, sct_ceil_log2<2> == 1, sct_ceil_log2<3> == 2, sct_ceil_log2<4> == 2
* ```sct_nbits<X>``` number of bits needed to store the value X: zero if X is zero; otherwise 1 + floor(log2(X)). E.g. sct_nbits<0> == 0, sct_nbits<1> == 1, sct_nbits<2> == 2, sct_nbits<3> == 2, sct_nbits<4> == 3
* ```sct_addrbits<N>``` address/index width for N elements, same as ceil(log2(N)). E.g. sct_addrbits<0> == 0, sct_addrbits<1> == 0, sct_addrbits<2> == 1, sct_addrbits<3> == 2, sct_addrbits<4> == 2
* ```sct_addrbits1<N>``` address/index width for N elements, same as sct_addrbits<N> but not less than one. E.g. sct_addrbits1<0> == 1, sct_addrbits1<1> == 1, sct_addrbits1<2> == 1, sct_addrbits1<3> == 2, sct_addrbits1<4> == 2
* ```sct_is_pow2<X>``` checks if a number X is power of 2, numbers 0 and 1 considered as power of 2. E.g. sct_is_pow2<0> == 1, sct_is_pow2<1> == 1, sct_is_pow2<8> == 1, sct_is_pow2<9> == 0
* ```sct_next_pow2<X>``` returns power of 2 number greater than X. E.g. sct_next_pow2<0> == 1, sct_next_pow2<1> == 2, sct_next_pow2<5> == 8, sct_is_pow2<8> == 16


# Floating/fixed point types

Floating and fixed points types not supported.




