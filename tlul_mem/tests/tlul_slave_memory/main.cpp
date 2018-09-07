/**
 * @author Canberk Sönmez
 * @file main.cpp
 * @brief Implements the testbench for Verilator.
 * 
 */


#define BOOST_TEST_MODULE __FILE__

#define FORCE_PRINT

#include <boost/test/unit_test.hpp>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <bitset>
#include <algorithm>

#include <random>
#include <chrono>

#include <verilator_aux.hpp>
#include <hdl_tests_tlul_slave_memory.h>
#include <verilated_vcd_c.h>

#include <type_traits>

#include "tlul_testbench.hpp"

namespace utf   = boost::unit_test::framework;

using memory_traits = verilator_aux::packed_traits<decltype(hdl_tests_tlul_slave_memory::DATA)>;
constexpr auto memory_size = memory_traits::size;

std::size_t main_time = 0;

double sc_time_stamp() {
    return main_time;
}

BOOST_AUTO_TEST_CASE(tlul_slave_memory) {
    auto const &argc = utf::master_test_suite().argc;
    auto const &argv = utf::master_test_suite().argv;
    
    Verilated::commandArgs(argc, argv);
    auto top = std::make_unique<hdl_tests_tlul_slave_memory>();
    auto tfp = std::make_unique<VerilatedVcdC>();
    
    tlul_testbench<hdl_tests_tlul_slave_memory> tb{top.get()};
    
    Verilated::traceEverOn(true);
    top->trace(tfp.get(), 99);
    tfp->open("dump.vcd");
    
    std::vector<uint8_t> generated;
    std::vector<uint8_t> acquired;
    
    
    auto continuous_puts1 = [&] {
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::mt19937 mt{seed};
        std::uniform_int_distribution<uint8_t> dist{0, 0xFF};
        
        for (decltype(tb)::address_type address = 0; address < memory_size; address += 8) {
            std::vector<uint8_t> data;
            data.reserve(8);
            for (int i = 0; i < 8; ++i) {
                auto d = dist(mt);
                data.push_back(d);
                generated.push_back(d);
            }
            
            tb.put_full_data([]{  }, address, 3, 0xff, std::move(data));
        }
    };
    
    auto continuous_reads1 = [&] {
        for (decltype(tb)::address_type address = 0; address < memory_size; address += 8) {
            tb.get([&](const std::vector<uint8_t> &v) {
                    std::copy(v.begin(), v.end(), std::back_inserter(acquired));
                }, address, 3, 0xff);
        }
    };
    
    auto continuous_reads2 = [&] {
        for (decltype(tb)::address_type address = 0; address < memory_size; address += 4) {
            tb.get([&](const std::vector<uint8_t> &v) {
                    std::copy(v.begin(), v.end(), std::back_inserter(acquired));
                }, address, 2, /* 0b11110000 */ 0b00001111);
        }
    };
    
    while (!Verilated::gotFinish()) {
        /**/ if (main_time == 10) {
            continuous_puts1();
        }
        else if (main_time == 1200) {
            continuous_reads1();
        }
        else if (main_time == 2000) {
            break;
        }
        
        main_time++;
        
        // toggle the clock
        top->CLK = !top->CLK;
        tb.eval();
        
        if (tfp) tfp->dump(main_time);
    }
    
    BOOST_TEST(acquired == generated);
    
    top->final();
    tfp->close();
}
