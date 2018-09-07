module tlul_uart_echo
    #(
        parameter
        CLKS_PER_BIT = 2,
        UART_ADDRESS = 127,
        W = 4,
        A = 32,
        Z = 4,
        O = 5,
        I = 5
    )
    (
        CLK,
        RX,
        TX
    );
    
    input CLK;
    input RX;
    output wire TX;
    
    wire [2:0]          a_opcode;
    wire [2:0]          a_param;
    wire [Z-1:0]        a_size;
    wire [O-1:0]        a_source;
    wire [A-1:0]        a_address;
    wire [W-1:0]        a_mask;
    wire [8*W-1:0]      a_data;
    wire                a_valid;
    wire                a_ready;
    
    wire [2:0]          d_opcode;
    wire [1:0]          d_param;
    wire [Z-1:0]        d_size;
    wire [O-1:0]        d_source;
    wire [I-1:0]        d_sink;
    wire [8*W-1:0]      d_data;
    wire                d_error;
    wire                d_valid;
    wire                d_ready;
    
    integer dummy;
    
    tlul_uart#(
        .CLKS_PER_BIT(CLKS_PER_BIT),
        .UART_ADDRESS(UART_ADDRESS),
        .W(W),
        .A(A),
        .Z(Z),
        .O(O),
        .I(I) ) m1(
            .CLK(CLK),
            .a_opcode(a_opcode),
            .a_param(a_param),
            .a_size(a_size),
            .a_source(a_source),
            .a_address(a_address),
            .a_mask(a_mask),
            .a_data(a_data),
            .a_valid(a_valid),
            .a_ready(a_ready),
            .d_opcode(d_opcode),
            .d_param(d_param),
            .d_size(d_size),
            .d_source(d_source),
            .d_sink(d_sink),
            .d_data(d_data),
            .d_error(d_error),
            .d_valid(d_valid),
            .d_ready(d_ready),
            .rx(RX),
            .tx(TX), 
            .INFO_CLKS_PER_BIT(dummy) );
    
    tlul_master_echo#(
        .UART_ADDRESS(UART_ADDRESS),
        .W(W),
        .A(A),
        .Z(Z),
        .O(O),
        .I(I) ) m2(
            .CLK(CLK),
            .a_opcode(a_opcode),
            .a_param(a_param),
            .a_size(a_size),
            .a_source(a_source),
            .a_address(a_address),
            .a_mask(a_mask),
            .a_data(a_data),
            .a_valid(a_valid),
            .a_ready(a_ready),
            .d_opcode(d_opcode),
            .d_param(d_param),
            .d_size(d_size),
            .d_source(d_source),
            .d_sink(d_sink),
            .d_data(d_data),
            .d_error(d_error),
            .d_valid(d_valid),
            .d_ready(d_ready) );
endmodule

