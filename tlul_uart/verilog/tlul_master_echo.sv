module tlul_master_echo
    #(
        parameter
        UART_ADDRESS = 127,
        W = 8,
        A = 32,
        Z = 4,
        O = 5,
        I = 5
    )
    (
        // the clock
        CLK,
        
        // BEGIN TL-UL Master Interface Ports
        
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
    );
    
    parameter BUFFER_SZ_LOG2 = $clog2(W);
    
    reg [8*W-1:0] buffer;
    
    input CLK;
    
    // BEGIN TL-UL Master Interface Port Definitions
    
    // Channel A definitions (for SLAVE interface)
    output reg [2:0]            a_opcode;
    output reg [2:0]            a_param;
    output reg [Z-1:0]          a_size;
    output reg [O-1:0]          a_source;
    output reg [A-1:0]          a_address;
    output reg [W-1:0]          a_mask;
    output reg [8*W-1:0]        a_data;
    output reg                  a_valid;
    input                       a_ready;
    
    // Channel D definitions (for SLAVE interface)
    input [2:0]                 d_opcode;
    input [1:0]                 d_param;
    input [Z-1:0]               d_size;
    input [O-1:0]               d_source;
    input [I-1:0]               d_sink;
    input [8*W-1:0]             d_data;
    input                       d_error;
    input                       d_valid;
    output reg                  d_ready;
    
    // END
    
    // BEGIN opcodes for TL-UL
    
    parameter OP_Get                = 3'd4;
    parameter OP_AccessAckData      = 3'd1;
    parameter OP_PutFullData        = 3'd0;
    parameter OP_PutPartialData     = 3'd1;
    parameter OP_AccessAck          = 3'd0;
    
    // END
    
    // BEGIN state definitions
    
    parameter st_IDLE   = 0;
    parameter st_WRDY   = 1;
    parameter st_WACK   = 2;
    
    // END
    
    reg [1:0] state;
    
    // BEGIN action definitions
    
    parameter act_None  = 0;
    parameter act_Read  = 1;
    parameter act_Write = 2;
    
    // END
    
    reg [1:0] action;
    
    initial begin
        d_ready = 0;
        a_opcode = 0;
        a_param = 0;
        a_size = 0;
        a_source = 0;
        a_address = 0;
        a_mask = 0;
        a_data = 0;
        a_valid = 0;
        
        state = st_IDLE;
        action = act_Read;
    end
    
    always @(posedge CLK) begin
        case (state)
        st_IDLE: begin
            if (action == act_None) begin
                // stay at st_IDLE
            end else begin
                case (action)
                act_Read: begin
                    // we need to put a Get operation of size BUFFER_SZ_LOG2
                    a_valid <= 1;
                    a_opcode <= OP_Get;
                    a_param <= 0;
                    /* verilator lint_off WIDTH */ a_size <= BUFFER_SZ_LOG2;
                    a_source <= 0;
                    a_address <= UART_ADDRESS;
                    a_mask <= {W{1'b1}};
                    a_data <= 0;
                    state <= st_WRDY;
                end
                act_Write: begin
                    a_valid <= 1;
                    a_opcode <= OP_PutFullData;
                    a_param <= 0;
                    /* verilator lint_off WIDTH */ a_size <= BUFFER_SZ_LOG2;
                    a_source <= 0;
                    a_address <= 127;
                    a_mask <= {W{1'b1}};
                    a_data <= buffer;
                    state <= st_WRDY;
                end
                default: begin end
                endcase
            end
        end
        st_WRDY: begin
            if (a_ready) begin
                a_valid <= 0;
                d_ready <= 1;
                
                state <= st_WACK;
            end
        end
        st_WACK: begin
            if (d_valid) begin
                case (action)
                act_Read: begin
                    buffer <= d_data;
                    d_ready <= 0;
                    state <= st_IDLE;
                    if (action == act_Read) action <= act_Write;
                    else if (action == act_Write) action <= act_Read;
                end
                act_Write: begin
                    d_ready <= 0;
                    state <= st_IDLE;
                    if (action == act_Read) action <= act_Write;
                    else if (action == act_Write) action <= act_Read;
                end
                default: begin end
                endcase
            end
        end
        endcase
    end
endmodule
