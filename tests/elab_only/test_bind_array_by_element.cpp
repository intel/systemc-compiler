//
// Created by ripopov on 11/27/18.
//

#include <systemc.h>

struct Y {
    int y;
};

struct Payload {
    int x[2];
    Y y;
    bool operator == (const Payload &other) const
    {
        return x[0] == other.x[0] && x[1] == other.x[1] && y.y == other.y.y;
    }
    friend std::ostream& operator << (std::ostream &os, const Payload &p) { return os; }
    friend void sc_trace(sc_trace_file *tf, const Payload & v, const std::string & NAME ) {}
};

SC_MODULE(array_to_array) {

    SC_MODULE (A) {

        SC_MODULE(inner) {
            sc_vector<sc_in<int>> invec{"invec",3};

            SC_CTOR(inner) {

            }
        };

        inner imod{"imod"};

        sc_vector<sc_in<int>> invec{"invec",3};
        sc_vector<sc_in<int>> invec2{"invec2",3};
        sc_vector<sc_in<int>> invec3{"invec3",3};

        SC_CTOR(A) {
            imod.invec(invec3);
        }
    };

    SC_MODULE(B) {
        sc_vector<sc_signal<int>> sigvec{"sigvec",3};
        SC_CTOR(B) {}
    };

    sc_vector<sc_signal<int>> sigvec{"sigvec",3};
    sc_vector<sc_signal<int>> sigvec2{"sigvec2",3};

    A a{"a"};
    B b{"b"};

    SC_CTOR (array_to_array) {
        a.invec(sigvec);
        a.invec2(b.sigvec);
        a.invec3(sigvec2);
    }

};


SC_MODULE(single_to_array) {

    SC_MODULE(A) {
        sc_in<Payload> in0{"in0"};
        sc_in<Payload> in1{"in1"};
//        sc_vector<sc_in<int>>   inputs{"signals", 2};

        SC_CTOR(A) {}
    };

    A a{"a"};

    sc_signal<int> sig_0{"sig_0"};
    sc_signal<int> sig_1{"sig_1"};
    sc_vector<sc_signal<Payload>>   signals{"signals", 2};

    SC_CTOR(single_to_array) {
        a.in0(signals[0]);
        a.in1(signals[1]);
//        a.inputs[0](sig_0);
//        a.inputs[1](sig_1);
    }

};

SC_MODULE (bind_arr_el) {

    SC_MODULE(SUB) {

        SC_MODULE(DEEPSUB) {
            sc_vector<sc_in<int>> in_vec{"in_vec", 2};
            SC_CTOR(DEEPSUB) {}
        } ds{"ds"};

        sc_vector<sc_out<int>> out_vec{"out_vec", 2};

        SC_CTOR(SUB) {
        }
    };

    SUB sub{"sub"};
    sc_vector<sc_signal<int>> sig_vec{"sig_vec", 2};

    sc_signal<int> sig0{"sig0"};
    sc_signal<int> sig1{"sig1"};

    SC_CTOR(bind_arr_el)
    {
        sub.ds.in_vec[0] (sig_vec[1]);
        sub.ds.in_vec[1] (sig_vec[0]);
        sub.out_vec[0]( sig0 );
        sub.out_vec[1]( sig1 );
    }

};

SC_MODULE (bind_arr_el_same) {

    sc_vector<sc_in<int>> in_vec{"in_vec",2};
    sc_vector<sc_out<int>> out_vec{"out_vec",2};
    sc_vector<sc_signal<int>> sig_vec{"sig_vec",2};

    SC_CTOR(bind_arr_el_same) {
        in_vec[0] (sig_vec[1]);
        in_vec[1] (sig_vec[0]);
        out_vec[0] (sig_vec[1]);
        out_vec[1] (sig_vec[0]);
    }

};


SC_MODULE (bind_down_arr_el) {

    SC_MODULE(SUB) {

        sc_signal<int> sub_sig{"sub_sig"};

        SC_MODULE(DEEP) {

            sc_vector<sc_signal<int>> deep_sig{"deep_sig",3};


            SC_CTOR(DEEP) {

            }

        } D {"D"};

        SC_CTOR(SUB) {

        }

    } S {"S"};

    sc_vector<sc_in<int>>  in_vec{"in_vec",2};
    sc_vector<sc_out<int>> out_vec{"out_vec",2};

    SC_CTOR(bind_down_arr_el) {

        in_vec[0](S.sub_sig);
        in_vec[1](S.D.deep_sig[0]);
        out_vec[0](S.D.deep_sig[1]);
        out_vec[1](S.D.deep_sig[2]);

    }

};


SC_MODULE(bind_arr_el_cross) {

    SC_MODULE (SUBA) {
        SC_CTOR(SUBA) {}
        sc_vector<sc_in<int>> ins{"ins",2};
        sc_vector<sc_out<int>> outs{"outs",2};
    } A{"A"};

    SC_MODULE (SUBB) {
        SC_CTOR(SUBB) {}

        SC_MODULE(DEEP) {
            SC_CTOR(DEEP) {}
            sc_vector<sc_signal<int>> sigs{"sigs",4};
        } D{"D"};

    } B{"B"};

    SC_CTOR (bind_arr_el_cross) {
        A.outs[0] (B.D.sigs[0]);
        A.outs[1] (B.D.sigs[1]);
        A.ins[0] (B.D.sigs[2]);
        A.ins[1] (B.D.sigs[3]);
    }

};

SC_MODULE(external_bind_el) {

    sc_in<bool> ins[2];

    SC_CTOR(external_bind_el) { }


};

SC_MODULE(top) {

    SC_CTOR(top) {
        by_element_external.ins[0](sig0);
    }

//    array_to_array a2a{"a2a"};
//    single_to_array s2a{"e2a"};
    bind_arr_el by_element{"by_element"};
    bind_arr_el_same by_element_same{"by_element_same"};
    bind_down_arr_el by_element_down{"by_element_down"};
    bind_arr_el_cross by_element_cross{"by_element_cross"};
    external_bind_el by_element_external{"by_element_external"};
    sc_signal<bool> sig0;
};

SC_MODULE(wrap) {
    top t{"t"};
    sc_signal<bool> sig0;
    SC_CTOR(wrap) {
        t.by_element_external.ins[1](sig0);
    }
};

int sc_main(int argc, char **argv) {
    wrap w{"w"};
    sc_start();
    return 0;
}