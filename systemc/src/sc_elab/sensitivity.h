//
// Created by ripopov on 9/28/18.
//

#ifndef SCTOOL_SENSITIVITY_H
#define SCTOOL_SENSITIVITY_H

namespace sc_core {
    class sc_process_b;
}

namespace sc_elab {

struct port_sens_proc {
    enum EVENT_KIND {DEFAULT, POSEDGE, NEGEDGE} kind;
    sc_core::sc_process_b * proc_ptr;
};

}

#endif //SCTOOL_SENSITIVITY_H
