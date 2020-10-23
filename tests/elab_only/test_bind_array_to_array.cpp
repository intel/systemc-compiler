//
// Created by ripopov on 12/12/18.
//

#include <systemc.h>

SC_MODULE(TEST) {
    SC_CTOR(TEST) {
        ivec[0].bind(sig);
    }

    sc_vector<sc_in<bool>> ivec{"ivec",1};
    sc_signal<bool> sig;

//    SC_MODULE(BIND_SAME) {
//        sc_vector <sc_signal<bool>> sig_vec{"sig_vec" , 2};
//        sc_vector <sc_in<bool>>     in_vec{"in_vec" , 2};
//
//        SC_CTOR(BIND_SAME) {
//            in_vec(sig_vec);
//        }
//
//    } BS{"BS"};
//
//    SC_MODULE(BIND_UP) {
//        SC_CTOR(BIND_UP) {
//            BUPS.BUPD.in_vec(sig_vec);
//        }
//
//        sc_vector <sc_signal<bool>> sig_vec{ "sig_vec" , 2 };
//
//        SC_MODULE(BIND_UP_SUB) {
//            SC_CTOR(BIND_UP_SUB) {
//                BUPD.out_arr[0](sig_vec[0]);
//                BUPD.out_arr[1](sig_vec[1]);
//            }
//
//            sc_vector<sc_signal<int>> sig_vec{ "sig_vec", 2 };
//
//            SC_MODULE(BIND_UP_DEEP) {
//                SC_CTOR(BIND_UP_DEEP) {}
//
//                sc_vector <sc_in<bool>> in_vec{ "in_vec" , 2 };
//                sc_out<int> out_arr[2];
//
//            } BUPD{"BUPD"};
//        } BUPS{"BUPS"};
//    } BUP{"BUP"};
//
//
//    SC_MODULE(BIND_CROSS) {
//        SC_CTOR(BIND_CROSS) {
//
//            A_INST.ivec(B_INST.sigvec);
//        }
//
//        SC_MODULE(A) {
//            SC_CTOR(A) {}
//
//            sc_vector<sc_in<unsigned >> ivec{"ivec",3};
//
//        } A_INST{"A_INST"};
//
//        SC_MODULE(B) {
//            SC_CTOR(B) {}
//
//            sc_vector<sc_signal<unsigned>> sigvec{"sigvec",3};
//
//        } B_INST{"B_INST"};
//
//    } BC{"BC"};

};


int sc_main(int argc, char **argv) {
    TEST t{"t"};
    sc_start();
    return 0;
}