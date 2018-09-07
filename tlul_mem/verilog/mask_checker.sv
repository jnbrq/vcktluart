module mask_checker
    #(
        parameter
        W = 8
    )
    (
        MASK,
        VALID
    );
    
    // mask_check works with a LUT (fast)
    input [W-1:0] MASK;
    output wire VALID;
    
    // generate the LUT
    genvar i, j;
    
    // that simple high school formula
    // unfortunately, that's all I can do :(
    localparam SZ = W*2-1; /* (2^z-1)/(2-1) */
    
    generate
        if (2 ** $clog2(W) != W) begin
            $error("sv-error: W must be a power of 2");
        end
        
        /* verilator lint_off UNOPTFLAT */ wire [SZ-1:0] wires;
        
        // note: I assume zero mask is not a mask at all
        for (i = 0; i <= $clog2(W); i = i + 1) begin: loop0
            for (j = 0; j < 2**($clog2(W)-i); j = j + 1) begin: loop1
                // localparam k = W*(2**i-1)/2**(i-1)+j;
                //                ^^^ this one does not work
                //                I guess it's a bug of Verilator
                // localparam k = (W * ((1 << i) - 1)) >> (i-1) + j;
                //                ^^^ this one does not work properly either
                //                ^^^ you need to rearrange parantheses
                //                ^^^ thanks to unit testing
                localparam k = ((W * ((1 << i) - 1)) >> (i-1)) + j;
                assign wires[k] = (MASK == {{(W-2**i){1'b0}}, {(2**i){1'b1}}} << j*2**i);
            end
        end
        
        assign VALID = wires > 0;
    endgenerate
    
    // for debugging purposes
    generate
    if (0) begin
    
    initial begin
        for (i = 0; i <= $clog2(W); i = i + 1) begin: loop0
            for (j = 0; j < 2**($clog2(W)-i); j = j + 1) begin: loop1
                $display("i = %d; j = %d; k = %d", i, j, ((W * ((1 << i) - 1)) >> (i-1)) + j);
            end
        end
    end
    
    end
    endgenerate
endmodule

