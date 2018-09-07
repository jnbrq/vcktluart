/**
 * @author Canberk Sönmez
 * @brief Contains some helper functions for Verilator.
 */

#ifndef VERILATOR_AUX_HPP_INCLUDED
#define VERILATOR_AUX_HPP_INCLUDED

#include <climits>
#include <type_traits>

#include <iostream>

namespace verilator_aux {

/**
 * @name packed_traits
 */
template <typename Base>
struct packed_traits: std::true_type {
    using base_type = Base;
    using element_type = Base;
    static constexpr auto size_in_bits = sizeof(Base) * CHAR_BIT;
    static constexpr auto size = sizeof(Base);
    static constexpr auto length = (unsigned) 1;
};

template <typename Base, unsigned M>
struct packed_traits<Base [M]>: std::true_type {
    using base_type = Base;
    using element_type = Base[M];
    static constexpr auto size_in_bits = sizeof(Base) * M * CHAR_BIT;
    static constexpr auto size = sizeof(Base) * M;
    static constexpr auto length = M;
};

template <typename Base, unsigned A, unsigned B>
struct packed_traits<Base [A][B]>: std::false_type {};

template <typename Base>
struct unpacked_traits2: std::false_type {};

template <typename Base, unsigned N>
struct unpacked_traits2<Base [N]>: std::true_type {
    using element_type = Base [N];
    using packed_traits = verilator_aux::packed_traits<Base>;
    
    static constexpr auto size_in_bits = packed_traits::size_in_bits * N;
    static constexpr auto size = packed_traits::size * N;
    static constexpr auto length = packed_traits::length * N;
};

template <typename Base, unsigned N, unsigned M>
struct unpacked_traits2<Base [N][M]>: std::true_type {
    using element_type = Base [N][M];
    using packed_traits = verilator_aux::packed_traits<Base [M]>;
    
    static constexpr auto size_in_bits = packed_traits::size_in_bits * N;
    static constexpr auto size = packed_traits::size * N;
    static constexpr auto length = packed_traits::length * N;
};

template <unsigned long N>
bool check_contiguous(std::bitset<N> bs) {
    // a contiguous bitset is
    //   000001111100000
    //   000000000000111
    //   111110000000000
    
    // so first, consume 0s, then consume 1s. if you are left with 0 only,
    // then yes, it is contiguous. otherwise not.
    
    // avoid inf loop
    if (bs.none())
        return true;
    
    // avoid inf loop
    if (bs.all())
        return true;
    
    while (bs[0] == false) {
        bs >>= 1;
    }
    
    while (bs[0] == true) {
        bs >>= 1;
    }
    
    return bs.none();
}

// Some helper functions for bit manipulation
template <typename T, typename N>
constexpr T get_byte(T t, N n) {
    // n * 8 = n << 3
    return (t & (T(0xff) << (n<<3))) >> (n<<3);
}

template <typename T, typename N>
constexpr T set_byte(T t, N n, std::uint8_t b) {
    return (t & ~(T(0xff) << (n<<3))) | (((uintmax_t) b) << (n<<3));
}

template <typename T, typename N>
constexpr T get_bit(T t, N n) {
    return (bool)(t & (T(1) << n));
}

template <typename T, typename N>
constexpr T set_bit(T t, N n, std::uint8_t b) {
    return (t & ~(T(1) << n)) | (((uintmax_t) b) << n);
}

};

#endif // VERILATOR_AUX_HPP_INCLUDED
