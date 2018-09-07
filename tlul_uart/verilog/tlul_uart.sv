module tlul_uart
    #(
        parameter
        // CLKS_PER_BIT = 87,
        CLKS_PER_BIT = 2,
        UART_ADDRESS = 127,
        W = 8,
        A = 32,
        Z = 4,
        O = 5,
        I = 5
    )
    (
        // the clock signal
        CLK,
        
        // BEGIN TL-UL Slave Interface Ports
        
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
        d_ready,
        
        // END
        
        // BEGIN UART Ports
        
        rx,
        tx,
        
        // END
        
        INFO_CLKS_PER_BIT
    );
    
    input CLK;
    
    // BEGIN TL-UL Slave Interface Port Definitions
    
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
    
    // BEGIN UART Port definitions
    
    input                   rx;
    output                  tx;
    
    // END
    
    output integer INFO_CLKS_PER_BIT = CLKS_PER_BIT;
    
    // BEGIN opcodes for TL-UL
    
    parameter OP_Get                = 3'd4;
    parameter OP_AccessAckData      = 3'd1;
    parameter OP_PutFullData        = 3'd0;
    parameter OP_PutPartialData     = 3'd1;
    parameter OP_AccessAck          = 3'd0;
    
    // END
    
    // BEGIN state definitions
    
    parameter st_IDLE = 0;
    parameter st_W1 = 1;
    parameter st_W2 = 2;
    parameter st_WTX = 3;
    parameter st_WRX = 4;
    parameter st_WRDY = 5;
    
    // END
    
    reg [2:0] state;
    
    wire            rx_dv;
    wire [7:0]      rx_byte;
    
    reg             tx_dv;
    reg [7:0]       tx_byte;
    wire            tx_active;
    wire            tx_done;
    
    reg [O-1:0]     source;
    reg [Z-1:0]     size;
    integer         sz;     // in bytes
    reg [W-1:0]     mask;
    
    uart_rx#(.CLKS_PER_BIT(CLKS_PER_BIT)) uart_rx1(
        .i_Clock(CLK),
        .i_Rx_Serial(rx),
        .o_Rx_DV(rx_dv),
        .o_Rx_Byte(rx_byte));
    
    uart_tx#(.CLKS_PER_BIT(CLKS_PER_BIT)) uart_tx1(
        .i_Clock(CLK),
        .i_Tx_DV(tx_dv),
        .i_Tx_Byte(tx_byte),
        .o_Tx_Active(tx_active),
        .o_Tx_Serial(tx),
        .o_Tx_Done(tx_done));
    
    // Here comes the non-trivial parts
    
    // When the data is present on a_data, it may not be
    // aligned to the beginning or even contiguous! This
    // possible non-contiguous arbitrarily located data
    // must be stored somewhere, and it is the intermediate_memory
    wire [8*W-1:0] intermediate_memory;
    masked_l2m_connector
        #( .W(W), .BYTE_BIT(8), .A(A) )
        l2m(
            .MASK_IN(a_mask),
            .MEM(intermediate_memory),
            .DATA(a_data));
    
    // when Put{Full,Partial}Data occurs, the information
    // is stored here. The lowest address corresponds to the
    // least significant bits and the storage is contiguous
    // (internal buffer)
    // also, received data is stored here
    reg [8*W-1:0] storage;
    integer index;
    
    
    wire [8*W-1:0] masked_d_data;
    masked_m2l_connector
        #( .W(W), .BYTE_BIT(8), .A(A) )
        m2l(
            .MASK_IN(mask),
            .MEM(storage),
            .DATA(masked_d_data));
    
    initial begin
        a_ready = 0;
        d_opcode = 0;
        d_param = 0;
        d_size = 0;
        d_source = 0;
        d_sink = 0;
        d_data = 0;
        d_error = 0;
        d_valid = 0;
        
        state = st_IDLE;
        tx_dv = 0;
        tx_byte = 0;
        
        source = 0;
        size = 0;
        mask = 0;
        
        storage = 0;
        index = 0;
    end
    
    always @(posedge CLK) begin
        case (state)
        st_IDLE: begin
            if (a_valid && a_address == UART_ADDRESS) begin
                source <= a_source;
                size <= a_size;
                sz <= (1 << a_size);
                
                a_ready <= 1'b1;
                
                case (a_opcode)
                OP_Get: begin
                    mask <= a_mask;
                    
                    state <= st_W2;
                end
                OP_PutFullData: begin
                    storage <= intermediate_memory;
                    
                    state <= st_W1;
                end
                OP_PutPartialData: begin
                    // TODO implement later
                    $display("sv-error: not implemented yet");
                end
                default: begin
                    // TODO Not implemented in TL-UL
                    $display("sv-error: not in TL-UL");
                end
                endcase
            end
        end
        st_W1: begin
            a_ready <= 1'b0;
            
            index <= 0;
            tx_dv <= 1'b1;
            tx_byte <= storage[7:0];
            
            state <= st_WTX;
        end
        st_WTX: begin
            // sequentially transmit bytes
            
            if (index == sz) begin
                // send AccessAck
                d_opcode <= OP_AccessAck;
                d_param <= 0;
                d_size <= size;
                d_valid <= 1'b1;
                d_source <= source;
                d_sink <= 0;
                d_data <= 0;
                d_error <= 0;
                
                state <= st_WRDY;
            end else if (tx_done) begin
                if (index + 1 == sz) begin
                    // change dv ASAP
                    tx_dv <= 1'b0;
                end
                index <= index + 1;
                tx_byte <= storage[((index+1) << 3) +: 8];
                
            end
        end
        st_W2: begin
            a_ready <= 1'b0;
            
            index <= 0;
            
            state <= st_WRX;
        end
        st_WRX: begin
            // sequentially receive bytes
            
            if (index == sz) begin
                // send AccessAckData
                a_ready <= 1'b1;
                d_opcode <= OP_AccessAckData;
                d_param <= 0;
                d_size <= size;
                d_valid <= 1'b1;
                d_source <= source;
                d_sink <= 0;
                d_data <= masked_d_data;
                d_error <= 0;
                
                state <= st_WRDY;
            end else if (rx_dv) begin
                storage[(index << 3) +: 8] <= rx_byte;
                index <= index + 1;
            end
        end
        st_WRDY: begin
            if (d_ready == 1'b1) begin
                d_valid <= 1'b0;
                
                // the followings are optional
                d_opcode <= 0;
                d_param <= 0;
                d_size <= 0;
                d_source <= 0;
                d_sink <= 0;
                d_data <= 0;
                d_error <= 0;
                d_valid <= 0;
                
                state <= st_IDLE;
            end
        end
        endcase
    end
endmodule
