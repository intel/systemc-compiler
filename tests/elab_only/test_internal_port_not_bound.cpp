#include "systemc.h"

// Internal module ports not bound
struct Child : sc_module 
{
    sc_in<bool> in;
    sc_out<bool> out;
    sc_in<int> in_[2];
    sc_in<int> in__[2][2];

    sc_in<bool>* in_p;
    sc_out<bool>* out_p;
    sc_in<int>* in_arr[2];
    sc_in<int>* in_arr2[2][2];

    SC_CTOR(Child) 
    {
        in_p = new sc_in<bool>("in_p");
        out_p = new sc_out<bool>("out_p");
        for (int i = 0; i < 2; i++) {
            in_arr[i] = new sc_in<int>("in_arr_name");
            for (int j = 0; j < 2; j++) {
                in_arr2[i][j] = new sc_in<int>("in_arr2_name");
            }
        }
        
        SC_METHOD(meth_proc);
        sensitive << in << *in_p;
    }
    
    void meth_proc() {
        out.write(in.read()); 
        out_p->write(in_p->read());
    }
};

struct Top : sc_module 
{
    Child child{"child"};
    
    SC_CTOR(Top) 
    {}
};

int sc_main(int argc, char** argv)
{
    sc_signal<bool> s{"s"};
    sc_signal<int> s_[2];
    sc_signal<int> s__[2][2];
    
    sc_signal<bool> ps{"ps"};
    sc_signal<int> ps_[2];
    sc_signal<int> ps__[2][2];

    Top top("top");
    
    top.child.in(s);
    top.child.out(s);
    top.child.in_p->bind(ps);
    top.child.out_p->bind(ps);
    for (int i = 0; i < 2; i++) {
        top.child.in_[i](s_[i]);
        top.child.in_arr[i]->bind(s_[i]);
        for (int j = 0; j < 2; j++) {
            top.child.in__[i][j](s__[i][j]);
            top.child.in_arr2[i][j]->bind(s__[i][j]);
    }}
            
    
    sc_start();
    return 0;
}
