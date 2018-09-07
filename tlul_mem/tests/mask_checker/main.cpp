#define BOOST_TEST_MODULE __FILE__

// #define FORCE_PRINT
// ^^^ uncomment to print each and every case, not only the errorneous ones

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <bitset>
#include <algorithm>

#include <hdl_tests_mask_checker_8bits.h>
#include <hdl_tests_mask_checker_64bits.h>

namespace utf   = boost::unit_test::framework;
using namespace std;

constexpr std::size_t size_limit = 1000000;

template <unsigned long N, typename HDL, typename T>
void test(T const *positives_base) {
    auto const &argc = utf::master_test_suite().argc;
    auto const &argv = utf::master_test_suite().argv;
    
    Verilated::commandArgs(argc, argv);
    
    // generate positives
    std::vector<T> positives;
    
    for (const T *mask_first = positives_base; *mask_first != 3; mask_first += 2) {
        auto const sz = *mask_first;
        auto mask = *(mask_first+1);
        if (sz) {
            for (unsigned i = 0; i < N/sz; ++i) {
                positives.push_back(mask);
                mask <<= sz;
            }
        }
        // zero mask is not a mask at all
    }
    
    auto is_valid = [&](T mask) {
        auto it = std::find(positives.begin(), positives.end(), mask);
        return it != positives.end();
    };
    
    // simulate
    auto top = std::make_unique<HDL>();
    
    std::size_t i = 0;
    
    for (T mask = 0x00; ; ++mask) {
        bool valid = is_valid(mask);
        
#ifdef FORCE_PRINT
        std::cout << std::noboolalpha;
        std::cout << "Current mask: " << std::bitset<N>(mask) << " ; VALID = " << valid;
#endif
        
        top->MASK = mask;
        top->eval();
        
#ifdef FORCE_PRINT
        std::cout << " ; OUTPUT = " << (bool) top->VALID << std::endl;
#endif
        
        if (valid != top->VALID) {
            std::cerr << "error case: " << std::bitset<N>(mask) << std::endl;
        }
        
        BOOST_CHECK_EQUAL(valid, top->VALID);
        
        if (mask == positives_base[1])
            break;
        
        ++i;
        if (i == size_limit) {
            std::cout << "limit reached.\n";
            break;
        }
    }
}

BOOST_AUTO_TEST_CASE(mask_checker_8bits) {
    uint8_t positives_base [] = {
        8, 0b1111'1111,
        4, 0b0000'1111,
        2, 0b0000'0011,
        1, 0b0000'0001,
        0, 0b0000'0000,
        3, 0
    };
    
    test<8, hdl_tests_mask_checker_8bits>(positives_base);
}

BOOST_AUTO_TEST_CASE(mask_checker_64bits) {
    uint64_t positives_base [] = {
        64, 0xFFFF'FFFF'FFFF'FFFF,
        32, 0x0000'0000'FFFF'FFFF,
        16, 0x0000'0000'0000'FFFF,
        8,  0b1111'1111,
        4,  0b0000'1111,
        2,  0b0000'0011,
        1,  0b0000'0001,
        0,  0b0000'0000,
        3, 0
    };
    
    test<64, hdl_tests_mask_checker_64bits>(positives_base);
}
