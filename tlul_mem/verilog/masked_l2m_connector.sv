module masked_l2m_connector
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
    /* verilator lint_off UNOPTFLAT */ wire [MLEN-1:0]      MEMS [W:0];
    output [MLEN-1:0] MEM;
    assign MEM = MEMS[W];
    input [DATA_LEN-1:0]        DATA;
    
    assign MEMS[0] = {MLEN{1'b0}};
    
    generate
        genvar i;
        
        /* verilator lint_off UNOPTFLAT */ wire [A-1:0] address_wires [W-1:0];
        
        // generate the masked_connector_units
        for (i = 0; i < W; i = i + 1) begin: loop0
            wire [MLEN-1:0] mem_tmp;
            
            if (i == 0) begin
                masked_l2m_connector_unit
                    #(
                        .W(W),
                        .BYTE_BIT(BYTE_BIT),
                        .A(A)
                    )
                    mcu
                    (
                        .MEM(mem_tmp),
                        .M(MASK_IN[i]),
                        .AIN(0),
                        .AOUT(address_wires[0]),
                        .DATA_LINE(DATA[BYTE_BIT-1:0])
                    );
            end else begin
                masked_l2m_connector_unit
                    #(
                        .W(W),
                        .BYTE_BIT(BYTE_BIT),
                        .A(A)
                    )
                    mcu
                    (
                        .MEM(mem_tmp),
                        .M(MASK_IN[i]),
                        .AIN(address_wires[i-1]),
                        .AOUT(address_wires[i]),
                        .DATA_LINE(DATA[i * BYTE_BIT +: BYTE_BIT])
                    );
            end
            
            assign MEMS[i+1] = mem_tmp | MEMS[i];
        end
    endgenerate
endmodule

module masked_l2m_connector_unit
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
    
    output reg [MLEN-1:0]       MEM;
    input                       M;
    input [A-1:0]               AIN;
    
    output wire [A-1:0]         AOUT;
    input [BYTE_BIT-1:0]        DATA_LINE;
    
    // replicated version of M
    wire [BYTE_BIT-1:0] MM = {BYTE_BIT{M}};
    
    assign AOUT                             = AIN + {{(A-1){1'b0}}, M};
    
    assign MEM = {{(MLEN-BYTE_BIT){1'b0}}, (MM & DATA_LINE)} << (AIN<<3);
endmodule

