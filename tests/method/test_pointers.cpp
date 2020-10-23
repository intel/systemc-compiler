#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Variable and channel pointers, arrays of channel pointers 
class A : public sc_module 
{
public:
    sc_signal<bool>     s{"s"};
    sc_signal<int>*     sp;
    sc_out<bool>        out{"out"};
    sc_out<bool>*       p;
    
    int                 m;
    int                 k;
    sc_uint<3>          n;
    sc_uint<3>          l;
    int*                q;
    int*                q2;
    
    int*                np;
    sc_signal<int>*     np2;
    
    sc_uint<3>*         u;
    sc_uint<3>*         u1;
    sc_uint<3>*         u2;
    
    static const unsigned N = 4;
    static const unsigned M = 5;
    sc_uint<12>*                parr1[N];
    sc_signal<sc_int<12>>*      parr2[N];
    sc_signal<sc_biguint<42>>** parrp;
    
    SC_CTOR(A)
    {
        p = new sc_out<bool>("p");
        sp = new sc_signal<int>("sp");

        q   = sc_new<int>();
        q2  = &k;
        u   = sc_new<sc_uint<3>>();
        u1  = sc_new<sc_uint<3>>();
        u2  = &n;
        np  = nullptr;
        np2 = nullptr;
        
        for (int i = 0; i < N; i++) {
            parr1[i] = sc_new<sc_uint<12>>();
            parr2[i] = new sc_signal<sc_int<12>>("parr2");
        }
        parrp = sc_new_array< sc_signal<sc_biguint<42>>* >(M);
        for (int i = 0; i < M; i++) {
            parrp[i] = new sc_signal<sc_biguint<42>>("parrp");
        }
        
        SC_METHOD(this_pointer);
        sensitive << s;
        
        SC_METHOD(this_pointer2);
        sensitive << *sp;

        SC_METHOD(pointer_decl_init);
        sensitive << out;

        SC_METHOD(pointer_if);
        sensitive << out;
        
        SC_METHOD(array_ptr1);
        sensitive << *sp;

        SC_METHOD(array_ptr2);
        for (int i = 0; i < N; i++) {
            sensitive << *parr2[i];
        }
    }
    
    // @this pointer dereference
    void this_pointer() 
    {
        this->m = 1;
        this->s = 2 + this->m;
        *this->u = 4 + this->s;
        this->sp->write(5 + *this->u);
        
        (*this).m = 6 + this->m;
        (*this).s = 7 + (*this).m;
        *(*this).u = 8 + (*this).s;
        (*this).sp->write(9 + *(*this).u);
    }
    
    void this_pointer2() {
        int i = (*(*this).sp).read() + this->sp->read();
    }
    
    // Pointer initialization at declaration
    void pointer_decl_init() 
    {
        int* p1 = q;
        int i = *p1 + *q;
        
        int* p2 = q2;
        i = *p2;
        
        sc_uint<3>* p3 = u1;
        i = *p3 - *u1 + *u2;
        
        sc_uint<3>* p4 = u2;
        *p4 = i + 1;
        i = *p4;
    }

    void pointer_if() 
    {
        if (np) {
            l = 1;
        } else {
            l = 2;
        }
        sct_assert_const(l == 2);
    }
    
// ----------------------------------------------------------------------------    
    
    void array_ptr1()
    {
        for (int i = 0; i < N; i++) {
            *parr1[i] = i;
        }

        for (int i = 0; i < N-1; i++) {
            (*parr2[i]).write(int(*parr1[i]));
        }
        *parr2[N-1] = *parr1[N-2] + *parr1[N-1];
        *parr2[sp->read()] = 1;
    }
    
    sc_signal<long int> sl;
    void array_ptr2()
    {
        for (int i = 0; i < N; i++) {
           *parrp[i] = (sc_biguint<42>)parr2[i]->read();
        }
        for (int i = N; i < M; i++) {
           *parrp[i] = 0;
        }
        sl = (sc_uint<32>)(parrp[0]->read() + parrp[parr2[0]->read()]->read()).to_long();
    }

};

class B_top : public sc_module {
public:
    sc_signal<bool>      s1;
    sc_signal<bool>      s2;

    A a_mod{"a_mod"};
    
    SC_CTOR(B_top) {
        a_mod.p->bind(s1);
        a_mod.out(s2);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
