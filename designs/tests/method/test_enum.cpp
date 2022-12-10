/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/22/18.
//

#include <systemc.h>
#include <sct_assert.h>

enum MY_MODE {
    OFF = 0,
    ON = 1
};

SC_MODULE(test_enum) {

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(test_enum) {
        SC_METHOD(test_method); sensitive << dummy;
    }

    void test_method() {
        COLOR xcolor;
        MY_MODE xmode;

        color = COLOR::RED;
        xcolor = color_const;
        xmode = mode_const;

        int icolor = BLUE;
        xcolor = BLUE;

        DIRECTIONS dir = DIRECTIONS::SOUTH;

        int x = 0;
        if (dir == DIRECTIONS::SOUTH)
            x = 1;
        else
            x = 2;

        sct_assert_const(x == 1);
        sct_assert_const(icolor == 3);
        sct_assert_const(color == COLOR::RED);
        sct_assert_const(dir == DIRECTIONS::SOUTH);
        sct_assert_const(nm == NUMBERS::THREE);

        cout << "done\n";
    }

    enum COLOR {
        RED = 1,
        GREEN,
        BLUE
    };

    enum class DIRECTIONS {
        NORTH,
        SOUTH,
        WEST,
        EAST
    };

    enum class NUMBERS: char {
        ONE = 1,
        TWO,
        THREE
    };
    const NUMBERS nm = NUMBERS :: THREE;

    const MY_MODE mode_const = MY_MODE::ON;
    MY_MODE mode = MY_MODE::OFF;

    COLOR color;

    const COLOR color_const = COLOR::GREEN;


};


int sc_main(int argc, char **argv) {
    auto tenum = std::make_unique<test_enum>("tenum");
    sc_start();
    return 0;
}
