module seq_shift(CLK, E, IN, OUT);
    input CLK;              // clock signal
    input E;                // active-high enable signal
    input [7:0] IN;         // 1 byte input
    output reg [7:0] OUT;   // 1 byte output
    
    // main logic
    always @(posedge CLK) begin
        if (E) begin
            OUT <= OUT << 1;    // if enabled, shift
        end else begin
            OUT <= IN;          // output the same otherwise
        end
    end
endmodule

