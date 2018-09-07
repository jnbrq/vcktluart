// include file of the Verilated Verilog module
#include <hdl_seq_shift.h>

// include file for trace recorder (for waveform display)
#include <verilated_vcd_c.h>

// provides std::make_unique<T>(...)
#include <memory>

// provides std::bitset<N> for bit-by-bit printing
#include <bitset>

// standard I/O
#include <iostream>

// for recording time
double main_time = -1;
double sc_time_stamp() { return main_time; }

int main(int argc, char **argv) {
    // let verilator to process command line arguments
    Verilated::commandArgs(argc, argv);
    
    // create a unique_ptr to the top level module
    auto top = std::make_unique<hdl_seq_shift>();
    
    // create a trace recorder object
    auto tfp = std::make_unique<VerilatedVcdC>();
    
    // enable trace recording
    Verilated::traceEverOn(true);
    top->trace(tfp.get(), 99);
    tfp->open("dump.vcd");
    
    // initial conditions
    top->E = 0;
    top->CLK = 1;
    top->IN = 0b1001'0110;
    
    // loop until simulation finishes
    while (!Verilated::gotFinish()) {
        if (main_time >= 25) break;     // limit simulation time
        ++main_time;                    // proceed to the future
        
        top->CLK = !top->CLK;           // toggle clock
        
        // if positive edge triggered
        if (top->CLK)  {
            // print (i.e. READ)
            std::cout << main_time << " " << std::bitset<8>(top->OUT) << std::endl;
        }
        
        top->eval();                    // evaluate the model
        
        // if positive edge triggered
        if (top->CLK) {
            if (main_time >= 3)
                top->E = 1;             // enable after a while (i.e. WRITE)
        }
        
        // force dump
        if (tfp) tfp->dump(main_time);
    }
    
    // finialize
    top->final();
    tfp->close();
    return 0;
}
