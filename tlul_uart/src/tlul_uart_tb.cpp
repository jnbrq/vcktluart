#include "tlul_testbench.hpp"
#include "uart_testbench.hpp"
#include <memory>

#include <verilated_vcd_c.h>
#include <hdl_tlul_uart.h>
#include <hdl_tlul_uart_echo.h>

double main_time = -1;

double sc_time_stamp() { return main_time; }

#if 0

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    
    auto top = std::make_unique<hdl_tlul_uart>();
    auto tfp = std::make_unique<VerilatedVcdC>();
    
    tlul_testbench<hdl_tlul_uart> tb{top.get()};
    
    Verilated::traceEverOn(true);
    top->trace(tfp.get(), 99);
    tfp->open("dump1.vcd");
    
    // Let's if we see some changes in UART
    tb.put_full_data([]() {}, 127, 0, 0b0000'0001, std::vector<std::uint8_t>{5});
    tb.put_full_data([]() {}, 127, 0, 0b0000'0001, std::vector<std::uint8_t>{9});
    tb.put_full_data([]() {}, 127, 2, 0b0000'1111, std::vector<std::uint8_t>{1, 2, 3, 4});
    tb.put_full_data([]() {}, 127, 3, 0b1111'1111, std::vector<std::uint8_t>{1, 2, 3, 4, 5, 6, 7, 8});
    
    top->CLK = 1;
    
    while (!Verilated::gotFinish()) {
        if (main_time == 20000) {
            break;
        }
        
        ++main_time;
        
        // toggle the clock
        top->CLK = !top->CLK;
        tb.eval();
        
        if (tfp) tfp->dump(main_time);
    }
    
    top->final();
    tfp->close();
    return 0;
}

#endif


#if 0
int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    
    auto top = std::make_unique<hdl_tlul_uart>();
    auto tfp = std::make_unique<VerilatedVcdC>();
    
    tlul_testbench<hdl_tlul_uart> tb{top.get()};
    auto uart_sender = uart::make_sender(&(top->CLK), &(top->rx), 2); // top->INFO_CLKS_PER_BIT);
    
    Verilated::traceEverOn(true);
    top->trace(tfp.get(), 99);
    tfp->open("dump2.vcd");
    
    top->CLK = 1;
    
    while (!Verilated::gotFinish()) {
        if (main_time == 20000) {
            break;
        }
        
        if (main_time == 5) {
            tb.get(
                [](auto const &v) { std::cout << (int)v[0] << " " << (int)v[1] << std::endl; },
                127, 1, 0b0000'0011);
        }
        else if (main_time == 20) {
            uart_sender.write_bytes("ca", [] {});
        }
        
        ++main_time;
        
        // toggle the clock
        top->CLK = !top->CLK;
        tb.eval();
        uart_sender.eval();
        
        if (tfp) tfp->dump(main_time);
    }
    
    top->final();
    tfp->close();
    return 0;
}

#endif

#if 0
int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    
    std::unique_ptr<hdl_tlul_uart> top{new hdl_tlul_uart};
    std::unique_ptr<VerilatedVcdC> tfp{new VerilatedVcdC};
    
    tlul_testbench<hdl_tlul_uart> tb{top.get()};
    auto uart_receiver = uart::make_receiver(
        [](std::uint8_t c) { std::cout << c << " "; },
        &(top->CLK), &(top->tx), 2); // top->INFO_CLKS_PER_BIT);
    
    Verilated::traceEverOn(true);
    top->trace(tfp.get(), 99);
    tfp->open("dump3.vcd");
    
    top->CLK = 1;
    
    while (!Verilated::gotFinish()) {
        if (main_time == 20000) {
            break;
        }
        
        if (main_time == 5) {
            tb.put_full_data(
                [] {}, 127, 2, 0b0000'1111, std::vector<std::uint8_t>{'c', 'a', 'n', 'b'});
        }
        
        ++main_time;
        
        // toggle the clock
        top->CLK = !top->CLK;
        uart_receiver.eval();
        tb.eval();
        
        if (tfp) tfp->dump(main_time);
    }
    
    top->final();
    tfp->close();
    return 0;
}

#endif


#if 1
int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    
    std::unique_ptr<hdl_tlul_uart_echo> top{new hdl_tlul_uart_echo};
    std::unique_ptr<VerilatedVcdC> tfp{new VerilatedVcdC};
    
    auto uart_receiver = uart::make_receiver(
        [](std::uint8_t c) { std::cout << c << " "; },
        &(top->CLK), &(top->TX), 2); // top->INFO_CLKS_PER_BIT);
    
    auto uart_sender = uart::make_sender(&(top->CLK), &(top->RX), 2);
    
    Verilated::traceEverOn(true);
    top->trace(tfp.get(), 99);
    tfp->open("dump4.vcd");
    
    top->CLK = 1;
    
    while (!Verilated::gotFinish()) {
        if (main_time == 20000) {
            break;
        }
        
        if (main_time == 5) {
            uart_sender.write_bytes("canberkxcanberkxcanberkx", [] {});
        }
        
        ++main_time;
        
        // toggle the clock
        top->CLK = !top->CLK;
        
        uart_receiver.eval();
        top->eval();
        uart_sender.eval();
        
        if (tfp) tfp->dump(main_time);
    }
    
    top->final();
    tfp->close();
    return 0;
}

#endif

