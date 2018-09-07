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

#include <verilator_aux.hpp>
#include <hdl_tests_masked_l2m_connector.h>
#include <hdl_tests_masked_m2l_connector.h>

namespace utf   = boost::unit_test::framework;
using namespace std;

// assumptions are
// please note that this test is hardcoded
// do not change the followings, unless you really want to modify the whole test
namespace params {
    constexpr auto W            = 8;
    constexpr auto BYTE_BIT     = 8;
};

using namespace verilator_aux;

template <typename T, typename Mask>
constexpr auto check(T mem, T data_line, Mask mask) {
    unsigned a = 0; // address at the memory
    for (unsigned n = 0; n < sizeof(Mask) * params::BYTE_BIT; ++n) {
        if (get_bit(mask, n)) {
            if (get_byte(data_line, n) != get_byte(mem, a))
                return false;
            ++a;
        }
    }
    return true;
}

BOOST_AUTO_TEST_CASE(masked_connector_golden) {
    // we have some random tests here to see if the C++ code really works
    
    BOOST_TEST(!get_bit<uint8_t>(0b00000000, 0));
    BOOST_TEST(!get_bit<uint8_t>(0b00000000, 1));
    BOOST_TEST(!get_bit<uint8_t>(0b00000000, 2));
    BOOST_TEST( get_bit<uint8_t>(0b00001000, 3));
    BOOST_TEST( get_bit<uint8_t>(0b00010000, 4));
    BOOST_TEST( get_bit<uint8_t>(0b00100000, 5));
    BOOST_TEST( get_bit<uint8_t>(0b01000000, 6));
    BOOST_TEST( get_bit<uint8_t>(0b10000000, 7));
    
    BOOST_CHECK_EQUAL(get_byte<uint64_t>(0xFFEEDDCC'99887766, 0), 0x66ul);
    BOOST_CHECK_EQUAL(get_byte<uint64_t>(0xFFEEDDCC'99887766, 1), 0x77ul);
    BOOST_CHECK_EQUAL(get_byte<uint64_t>(0xFFEEDDCC'99887766, 2), 0x88ul);
    BOOST_CHECK_EQUAL(get_byte<uint64_t>(0xFFEEDDCC'99887766, 3), 0x99ul);
    BOOST_CHECK_EQUAL(get_byte<uint64_t>(0xFFEEDDCC'99887766, 4), 0xCCul);
    BOOST_CHECK_EQUAL(get_byte<uint64_t>(0xFFEEDDCC'99887766, 5), 0xDDul);
    BOOST_CHECK_EQUAL(get_byte<uint64_t>(0xFFEEDDCC'99887766, 6), 0xEEul);
    BOOST_CHECK_EQUAL(get_byte<uint64_t>(0xFFEEDDCC'99887766, 7), 0xFFul);
    
    BOOST_CHECK_EQUAL(set_byte<uint64_t>(0xFFEEDDCC'99887766, 0, 0x10), 0xFFEEDDCC'99887710);
    BOOST_CHECK_EQUAL(set_byte<uint64_t>(0xFFEEDDCC'99887766, 1, 0x10), 0xFFEEDDCC'99881066);
    BOOST_CHECK_EQUAL(set_byte<uint64_t>(0xFFEEDDCC'99887766, 2, 0x10), 0xFFEEDDCC'99107766);
    BOOST_CHECK_EQUAL(set_byte<uint64_t>(0xFFEEDDCC'99887766, 3, 0x10), 0xFFEEDDCC'10887766);
    BOOST_CHECK_EQUAL(set_byte<uint64_t>(0xFFEEDDCC'99887766, 4, 0x10), 0xFFEEDD10'99887766);
    BOOST_CHECK_EQUAL(set_byte<uint64_t>(0xFFEEDDCC'99887766, 5, 0x10), 0xFFEE10CC'99887766);
    BOOST_CHECK_EQUAL(set_byte<uint64_t>(0xFFEEDDCC'99887766, 6, 0x10), 0xFF10DDCC'99887766);
    BOOST_CHECK_EQUAL(set_byte<uint64_t>(0xFFEEDDCC'99887766, 7, 0x10), 0x10EEDDCC'99887766);
    
    BOOST_TEST( (check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x00000000'00000000, 0b00000000)));
    BOOST_TEST( (check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x00000000'00000066, 0b00000001)));
    BOOST_TEST( (check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x00000000'00007766, 0b00000011)));
    BOOST_TEST( (check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x00000000'00006600, 0b00000010)));
    BOOST_TEST( (check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x00000000'00661100, 0b00000100)));
    BOOST_TEST( (check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x77000000'00666600, 0b10000100)));
    BOOST_TEST( (check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x77000000'00666600, 0b10000100)));
    BOOST_TEST( (check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0xFFEEDDCC'99887766, 0b11111111)));
    
    
    BOOST_TEST(!(check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x00000000'0000006F, 0b00000001)));
    BOOST_TEST(!(check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x00000000'0000776F, 0b00000011)));
    BOOST_TEST(!(check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x00000000'00006F00, 0b00000010)));
    BOOST_TEST(!(check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x00000000'006F1100, 0b00000100)));
    BOOST_TEST(!(check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x77000000'006F6600, 0b10000100)));
    BOOST_TEST(!(check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0x77000000'006F6600, 0b10000100)));
    BOOST_TEST(!(check<uint64_t, uint8_t>(0xFFEEDDCC'99887766, 0xFFEEDDCC'9988776F, 0b11111111)));
}

BOOST_AUTO_TEST_CASE(masked_m2l_connector) {
    auto const &argc = utf::master_test_suite().argc;
    auto const &argv = utf::master_test_suite().argv;
    
    Verilated::commandArgs(argc, argv);
    
    auto top = std::make_unique<hdl_tests_masked_m2l_connector>();
    
    // first, set the memory accordingly
    for (unsigned i = 0; i < params::W; ++i) {
        top->MEM = set_byte(top->MEM, i, i);
    }
    
    // now, for each mask,
    for (unsigned i = 0; i <= 0xff; ++i) {
        uint8_t mask = i;
        
        top->DATA = 0;
        top->MASK_IN = mask;
        top->eval();
        
#ifdef FORCE_PRINT
        std::cout << "Current mask: " << std::bitset<8>(i) << std::endl;
#endif
        
        BOOST_TEST(check(top->MEM, top->DATA, mask));
    }
}

BOOST_AUTO_TEST_CASE(masked_l2m_connector) {
    auto const &argc = utf::master_test_suite().argc;
    auto const &argv = utf::master_test_suite().argv;
    
    Verilated::commandArgs(argc, argv);
    
    auto top = std::make_unique<hdl_tests_masked_l2m_connector>();
    
    // first, set the  accordingly
    for (unsigned i = 0; i < params::W; ++i) {
        top->DATA = set_byte(top->DATA, i, i);
    }
    
    // now, for each mask,
    for (unsigned i = 0; i <= 0xff; ++i) {
        uint8_t mask = i;
        
        top->MEM = 0;
        top->MASK_IN = mask;
        top->eval();
        
#ifdef FORCE_PRINT
        std::cout << "Current mask: " << std::bitset<8>(i) << std::endl;
#endif
        
        BOOST_TEST(check(top->MEM, top->DATA, mask));
    }
}
