/**
 * @author Canberk Sönmez
 * @file tlul_slave_memory.sv
 * @brief Implements TileLink-UL protocol as slave agent.
 * 
 */

// tlul_slave_memory module
module tlul_slave_memory
    #(
        // BEGIN Parameters
        parameter
        RAM_SIZE = 256, // bytes
        W = 8,
        A = 32,
        Z = 4,
        O = 5,
        I = 5
        // END
    )
    (
        // BEGIN Port Declarations
        
        // the clock signal
        CLK,
        
        // the reset signal
        RESET,
        
        // the data contained in the RAM
        // only for debugging purposes
        DATA,
        
        // Channel A ports
        a_opcode,
        a_param,
        a_size,
        a_source,
        a_address,
        a_mask,
        a_data,
        a_valid,
        a_ready,
        
        // Channel D ports
        d_opcode,
        d_param,
        d_size,
        d_source,
        d_sink,
        d_data,
        d_error,
        d_valid,
        d_ready
        
        // END
        
        , DATA_DBG
    );
    
    // BEGIN Port Definitions
    
    input CLK;
    input RESET;
    
    output reg [8*RAM_SIZE-1:0] DATA;
    
    output wire [63:0] DATA_DBG = DATA[63:0];
    
    // Channel A definitions (for SLAVE interface)
    input [2:0]             a_opcode;
    input [2:0]             a_param;
    input [Z-1:0]           a_size;
    input [O-1:0]           a_source;
    input [A-1:0]           a_address;
    input [W-1:0]           a_mask;
    input [8*W-1:0]         a_data;
    input                   a_valid;
    output reg              a_ready;
    
    // Channel D definitions (for SLAVE interface)
    output reg [2:0]        d_opcode;
    output reg [1:0]        d_param;
    output reg [Z-1:0]      d_size;
    output reg [O-1:0]      d_source;
    output reg [I-1:0]      d_sink;
    output reg [8*W-1:0]    d_data;
    output reg              d_error;
    output reg              d_valid;
    input                   d_ready;
    
    // END
    
    // BEGIN opcodes for TL-UL
    parameter OP_Get                = 3'd4;
    parameter OP_AccessAckData      = 3'd1;
    parameter OP_PutFullData        = 3'd0;
    parameter OP_PutPartialData     = 3'd1;
    parameter OP_AccessAck          = 3'd0;
    // END
    
    parameter st_IDLE = 0;
    parameter st_WRDY = 1;
    
    reg [3:0] state = st_IDLE;
    
    // for debugging purposes
    integer i;
    integer tmp;
    
    initial begin
        for (i = 0; i < RAM_SIZE; i = i + 1) begin: test
            tmp = i ; // *8;
            DATA[i*8 +: 8] = tmp[7:0];
        end
    end
    
    parameter ROWBITS = 4;
    reg [ROWBITS-1:0] temp;
    
    wire [8*W-1:0] masked_d_data;
    masked_m2l_connector
        #( .W(W), .BYTE_BIT(8), .A(A) )
        m2l(
            .MASK_IN(a_mask),
            .MEM(DATA[a_address * 8 +: W*8]),
            .DATA(masked_d_data));
    
    wire [8*W-1:0] masked_memory;
    
    // I do not have a better idea
    // not liked this.
    reg [8*W-1:0] intermediate_memory;
    masked_l2m_connector
        #( .W(W), .BYTE_BIT(8), .A(A) )
        l2m(
            .MASK_IN(a_mask),
            .MEM(intermediate_memory),
            .DATA(a_data));
    
    reg [8*W-1:0] mask_size;
    mask_by_size
        #( .W(W), .Z(Z), .BYTE_BIT(8) )
        mbs( .SIZE(a_size), .MASK(mask_size) );
    
    always @(posedge CLK) begin
        case (state)
        st_IDLE: begin
            if (a_valid == 1'b1) begin
                case (a_opcode)
                OP_Get: begin
                    a_ready <= 1'b1;
                    d_opcode <= OP_AccessAckData;
                    d_param <= 0;
                    d_size <= a_size;   // we can still use a_size, since a_valid == 1
                    d_valid <= 1'b1;
                    d_source <= a_source;
                    d_sink <= 0;
                    d_data <= masked_d_data;
                    d_error <= 0;
                    
                    state <= st_WRDY;
                end
                OP_PutFullData: begin
                    a_ready <= 1'b1;
                    d_opcode <= OP_AccessAck;
                    d_param <= 0;
                    d_size <= a_size;   // we can still use a_size, since a_valid == 1
                    d_valid <= 1'b1;
                    d_source <= a_source;
                    d_sink <= 0;
                    d_data <= 0;
                    d_error <= 0;
                    
                    DATA[a_address*8 +: W*8] <=
                        (intermediate_memory & mask_size) |
                        (DATA[a_address*8 +: W*8] & (~mask_size));
                    
                    state <= st_WRDY;
                end
                default: begin
                    $error("sv-error: not implemented");
                    $finish;
                end
                endcase
            end
        end
        st_WRDY: begin
            if (d_ready == 1'b1) begin
                d_valid <= 1'b0;
                a_ready <= 1'b0;
                
                state <= st_IDLE;
            end
        end
        endcase
    end
endmodule
