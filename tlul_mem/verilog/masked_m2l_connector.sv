module masked_m2l_connector
    #(
        parameter
        W = 8,
        BYTE_BIT = 8,
        A = 32
    )
    (
        MASK_IN,
        MEM,
        DATA
    );
    
    localparam MLEN     = W*BYTE_BIT;
    localparam DATA_LEN = W*BYTE_BIT;
    
    input [W-1:0]               MASK_IN;
    input [MLEN-1:0]            MEM;
    output wire [DATA_LEN-1:0]  DATA;
    
    generate
        genvar i;
        
        /* verilator lint_off UNOPTFLAT */ wire [A-1:0] address_wires [W-1:0];
        
        // generate the masked_connector_units
        for (i = 0; i < W; i = i + 1) begin: loop0
            if (i == 0) begin
                masked_m2l_connector_unit
                    #(
                        .W(W),
                        .BYTE_BIT(BYTE_BIT),
                        .A(A)
                    )
                    mcu
                    (
                        .MEM(MEM),
                        .M(MASK_IN[i]),
                        .AIN(0),
                        .AOUT(address_wires[0]),
                        .DATA_LINE(DATA[BYTE_BIT-1:0])
                    );
            end else begin
                masked_m2l_connector_unit
                    #(
                        .W(W),
                        .BYTE_BIT(BYTE_BIT),
                        .A(A)
                    )
                    mcu
                    (
                        .MEM(MEM),
                        .M(MASK_IN[i]),
                        .AIN(address_wires[i-1]),
                        .AOUT(address_wires[i]),
                        .DATA_LINE(DATA[i * BYTE_BIT +: BYTE_BIT])
                    );
            end
        end
    endgenerate
endmodule

module masked_m2l_connector_unit
    #(
        parameter
        W = 8,  // mask length
        BYTE_BIT = 8,
        A = 32  // address length
    )
    (
        // memory
        MEM,
        
        // mask bit
        M,
        
        // address input (0 for initial, increases by BYTE_W)
        AIN,
        
        // address output
        AOUT,
        
        // corresponding data lane
        DATA_LINE
    );
    
    localparam                  MLEN = W*BYTE_BIT;
    
    input [MLEN-1:0]            MEM;
    input                       M;
    input [A-1:0]               AIN;
    
    output wire [A-1:0]         AOUT;
    output wire [BYTE_BIT-1:0]  DATA_LINE;
    
    // replicated version of M
    wire [BYTE_BIT-1:0] MM = {BYTE_BIT{M}};
    
    assign AOUT                 = AIN + {{(A-1){1'b0}}, M};
    assign DATA_LINE            = MM & MEM[AIN*BYTE_BIT +: BYTE_BIT];
endmodule

