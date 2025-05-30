//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.17
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: B_top ()
//
module B_top // "b_mod"
(
);

// Variables generated for SystemC signals
logic a;
logic b;
logic c;


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .a(a),
  .b(b),
  .c(c)
);

endmodule



//==============================================================================
//
// Module: A (test_switch_inside.cpp:415:5)
//
module A // "b_mod.a_mod"
(
    input logic a,
    output logic b,
    output logic c
);

// Variables generated for SystemC signals
logic [2:0] s;
logic [2:0] t;

// Local parameters generated for C++ constants
localparam logic ONE = 1;
localparam logic ZERO = 0;

//------------------------------------------------------------------------------
// Method process: switch_if1 (test_switch_inside.cpp:58:5) 

// Process-local variables
logic signed [31:0] k;

always_comb 
begin : switch_if1     // test_switch_inside.cpp:58:5
    integer i;
    if (|s)
    begin
        case (t)
        1 : begin
            i = 1;
        end
        2 : begin
            i = 2;
        end
        default : begin
            k = 3;
        end
        endcase
    end
    i = 0;
end

//------------------------------------------------------------------------------
// Method process: switch_if2 (test_switch_inside.cpp:70:5) 

always_comb 
begin : switch_if2     // test_switch_inside.cpp:70:5
    integer i;
    if (|s)
    begin
    end else begin
        case (t)
        1 : begin
            i = 1;
        end
        2 : begin
            i = 2;
        end
        default : begin
        end
        endcase
    end
    i = 0;
end

//------------------------------------------------------------------------------
// Method process: switch_if3 (test_switch_inside.cpp:83:5) 

always_comb 
begin : switch_if3     // test_switch_inside.cpp:83:5
    integer i;
    integer m_1;
    m_1 = s;
    if (|s || |t)
    begin
        case (m_1)
        1 : begin  // Empty case without break
            i = 2;
        end
        2 : begin
            i = 2;
        end
        endcase
    end
    i = 0;
end

//------------------------------------------------------------------------------
// Method process: switch_if4 (test_switch_inside.cpp:95:5) 

always_comb 
begin : switch_if4     // test_switch_inside.cpp:95:5
    integer i;
    if (|s)
    begin
        if (|t)
        begin
            case (a)
            1 : begin  // Empty case without break
                i = 2;
            end
            default : begin
                i = 2;
            end
            endcase
        end
        case (i)
        1 : begin
            i = 2;
        end
        2 : begin
            if (a)
            begin
            end
            i = 3;
        end
        endcase
    end
    case (s)
    1 : begin
        i = 2;
    end
    endcase
end

//------------------------------------------------------------------------------
// Method process: switch_if5 (test_switch_inside.cpp:117:5) 

always_comb 
begin : switch_if5     // test_switch_inside.cpp:117:5
    integer i;
    if (|s)
    begin
    end else begin
        case (i)
        1 : begin
            i = 2;
        end
        2 : begin
            i = 3;
        end
        default : begin
            while (i < 3)
            begin
                i++;
            end
        end
        endcase
        if (|t)
        begin
            case (s)
            1 : begin
                i = 1;
                if (|s)
                begin
                    i--;
                end
            end
            default : begin
                i = 2;
            end
            endcase
        end
    end
    if (|s)
    begin
        i = 1;
    end
end

//------------------------------------------------------------------------------
// Method process: switch_if_comp1 (test_switch_inside.cpp:144:5) 

// Process-local variables
logic arr[3];

always_comb 
begin : switch_if_comp1     // test_switch_inside.cpp:144:5
    integer i;
    if (ZERO || |s)
    begin
        case (i)
        1 : begin
            i = 2;
        end
        2 : begin
            i = 3;
        end
        default : begin
        end
        endcase
        if (1)
        begin
            i++;
        end
    end
    if (ONE && |t)
    begin
        case (i)
        1 : begin
            for (integer i_1 = 0; i_1 < 3; ++i_1)
            begin
                arr[i_1] = a;
            end
        end
        2 : begin
            i = 3;
        end
        endcase
    end
end

//------------------------------------------------------------------------------
// Method process: switch_if_comp2 (test_switch_inside.cpp:169:5) 

always_comb 
begin : switch_if_comp2     // test_switch_inside.cpp:169:5
    integer i;
    if (0)
    begin
    end
    if (1)
    begin
        case (i)
        1 : begin
            if (ZERO || a)
            begin
                i++;
            end
        end
        endcase
    end
end

//------------------------------------------------------------------------------
// Method process: switch_for1 (test_switch_inside.cpp:192:5) 

always_comb 
begin : switch_for1     // test_switch_inside.cpp:192:5
    integer i;
    integer l;
    l = s;
    for (integer j = 0; j < 7; j++)
    begin
        case (l)
        1 : begin
            i = 1;
        end
        2 : begin
            i = 2;
        end
        endcase
    end
end

//------------------------------------------------------------------------------
// Method process: switch_for2 (test_switch_inside.cpp:202:5) 

always_comb 
begin : switch_for2     // test_switch_inside.cpp:202:5
    integer i;
    integer m_1;
    m_1 = s;
    for (integer j = 0; j < 7; j++)
    begin
        case (s)
        1 : begin
            i = 1;
        end
        2 : begin
            i = 2;
        end
        default : begin
        end
        endcase
        case (m_1)
        1 : begin
            i = 1;
        end
        default : begin
            if (|s)
            begin
                i = 2;
            end
        end
        endcase
    end
end

//------------------------------------------------------------------------------
// Method process: switch_for3 (test_switch_inside.cpp:219:5) 

always_comb 
begin : switch_for3     // test_switch_inside.cpp:219:5
    integer i;
    integer m_1;
    m_1 = s;
    for (integer j = 0; j < 3; j++)
    begin
        if (|t)
        begin
            case (s)
            1 : begin  // Empty case without break
                if (|s)
                begin
                    i = 1;
                end
            end
            2 : begin
                if (|s)
                begin
                    i = 1;
                end
            end
            default : begin
            end
            endcase
        end
        for (integer l = 0; l < 3; l++)
        begin
            case (m_1)
            1 : begin
                i = 1;
            end
            default : begin
                i = 2;
            end
            endcase
        end
    end
end

//------------------------------------------------------------------------------
// Method process: switch_for4 (test_switch_inside.cpp:240:5) 

always_comb 
begin : switch_for4     // test_switch_inside.cpp:240:5
    integer i;
    integer m_1;
    m_1 = s;
    for (integer j = 0; j < 3; j++)
    begin
        if (|t)
        begin
            case (s)
            1 : begin
                i = 1;
            end
            default : begin
                for (integer l = 0; l < 3; l++)
                begin
                    case (m_1)
                    1 : begin
                        i = 1;
                    end
                    default : begin
                        i = 2;
                    end
                    endcase
                end
            end
            endcase
        end
    end
end

//------------------------------------------------------------------------------
// Method process: switch_for5 (test_switch_inside.cpp:259:5) 

// Process-local variables
logic [3:0] arr2d[3][4];

always_comb 
begin : switch_for5     // test_switch_inside.cpp:259:5
    integer i;
    logic [15:0] sum;
    i = 0;
    sum = 1;
    for (integer j = 0; j < 3; j++)
    begin
        for (integer l = 0; l < 4; l++)
        begin
            case (arr2d[j][l])
            1 : begin
                i = 1;
            end
            default : begin
                i = 2;
            end
            endcase
            sum = sum + i;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: switch_for6 (test_switch_inside.cpp:273:5) 

always_comb 
begin : switch_for6     // test_switch_inside.cpp:273:5
    integer i;
    for (integer j = 1; j < 3; j++)
    begin
        if (1)
        begin
            case (s)
            2 : begin
                i = 1;
            end
            endcase
        end
    end
    for (integer j_1 = 1; j_1 < 3; j_1++)
    begin
        case (s)
        3 : begin
            i = 1;
        end
        default : begin
        end
        endcase
    end
end

//------------------------------------------------------------------------------
// Method process: switch_while1 (test_switch_inside.cpp:293:5) 

always_comb 
begin : switch_while1     // test_switch_inside.cpp:293:5
    integer i;
    integer j;
    i = 0;
    j = 0;
    while (j < 4)
    begin
        if (j == s)
        begin
            case (t)
            1 : begin
                i = 1;
            end
            default : begin
                i = 2;
            end
            endcase
        end
        j = j + 2;
    end
end

//------------------------------------------------------------------------------
// Method process: switch_while2 (test_switch_inside.cpp:307:5) 

always_comb 
begin : switch_while2     // test_switch_inside.cpp:307:5
    integer i;
    integer j;
    i = 0;
    j = 5;
    while (j != 0)
    begin
        case (t)
        1 : begin
            for (integer j_1 = 1; j_1 < 3; j_1++)
            begin
                i = i + j_1;
            end
        end
        default : begin
            i = 2;
        end
        endcase
        j--;
    end
end

//------------------------------------------------------------------------------
// Method process: switch_call0 (test_switch_inside.cpp:334:5) 

always_comb 
begin : switch_call0     // test_switch_inside.cpp:334:5
    integer i;
    i = 1;
end

//------------------------------------------------------------------------------
// Method process: switch_call1 (test_switch_inside.cpp:351:5) 

always_comb 
begin : switch_call1     // test_switch_inside.cpp:351:5
    integer i;
    logic [3:0] TMP_2;
    logic [3:0] val;
    i = 0;
    i = 2;
    i = 1;
    val = t;
    // Call swfunc1() begin
    case (val)
    1 : begin
        TMP_2 = 1;
    end
    2 : begin
        if (val == s)
        begin
            val = val + 1;
        end
        TMP_2 = val;
    end
    default : begin
        TMP_2 = val + 2;
    end
    endcase
    // Call swfunc1() end
    i = TMP_2;
end

//------------------------------------------------------------------------------
// Method process: switch_call2 (test_switch_inside.cpp:378:5) 

always_comb 
begin : switch_call2     // test_switch_inside.cpp:378:5
    integer i;
    logic [3:0] TMP_0;
    logic [3:0] val;
    integer l;
    logic [3:0] TMP_3;
    integer TMP_1;
    integer val_1;
    i = 0;
    val = 0;
    // Call swfunc2() begin
    l = 0;
    case (0)
    1 : begin
        l = 1;
    end
    2 : begin
        if (val == s)
        begin
            l = 2;
        end
    end
    default : begin
        l = 2;    // Call of f()
    end
    endcase
    TMP_0 = l;
    // Call swfunc2() end
    i = TMP_0;
    i = 1;
    val = t;
    // Call swfunc2() begin
    l = 0;
    case (val)
    1 : begin
        l = 1;
    end
    2 : begin
        if (val == s)
        begin
            l = 2;
        end
    end
    default : begin
        val_1 = val + 1;    // Call of f()
        // Call f() begin
        TMP_1 = val_1 + 1;
        // Call f() end
        l = TMP_1;    // Call of f()
    end
    endcase
    TMP_3 = l;
    // Call swfunc2() end
    i = TMP_3;
end

endmodule


