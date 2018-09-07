module mask_by_size
    #( parameter W = 8, Z = 4, BYTE_BIT = 8 )
    ( SIZE, MASK );
    
    input [Z-1:0] SIZE;
    output reg [BYTE_BIT*W-1:0] MASK;
    
    generate
        genvar i;
        
        for (i = 0; i < W; i = i + 1) begin
            assign MASK[i*BYTE_BIT +: BYTE_BIT] = {BYTE_BIT{i < (1<<SIZE)}};
        end
    endgenerate
    
endmodule

