/**
 * @author Canberk Sönmez
 * @file tlul_testbench.hpp
 * @brief Contains tlul_testbench stuff.
 */

#ifndef TLUL_TESTBENCH_INCLUDED
#define TLUL_TESTBENCH_INCLUDED

#include <queue>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <bitset>

#include <boost/variant.hpp>

#include "verilator_aux.hpp"

/**
 * At first I will not consider the case with multiple
 * operations at the same time. Next operation is dispatched
 * iff the previous operation returned an ACK.
 * 
 */

namespace detail {

using namespace verilator_aux;

// I might've misunderstood, forget this
template <unsigned long N>
bool check_correct_mask(std::bitset<N> bs) {
    if (bs.none())
        return false;
    if (bs.all())
        return true;
    
    unsigned n = 0;
    
    while (bs[0] == false) {
        ++n;
        bs >>= 1;
    }
    
    unsigned k = 0;
    
    while (bs[0] == true) {
        ++k;
        bs >>= 1;
    }
    
    // check if k is a multiple of 2
    if (k && !(k & (k - 1))) {
        if (n % k != 0) {
            return false;
        }
    }
    else {
        return false;
    }
    
    // must be contiguous
    return bs.none();
}

template <typename HDLSlaveMemory>
struct tlul_testbench: boost::static_visitor<bool> {
    
    tlul_testbench(HDLSlaveMemory *hdlslavememory):
        hdl {hdlslavememory} {
    }
    
    using address_traits    = packed_traits<decltype(HDLSlaveMemory::a_address)>;
    using size_traits       = packed_traits<decltype(HDLSlaveMemory::a_size)>;
    using mask_traits       = packed_traits<decltype(HDLSlaveMemory::a_mask)>;
    using data_traits       = packed_traits<decltype(HDLSlaveMemory::a_data)>;
    
    using address_type      = typename address_traits::element_type;
    using size_type         = typename size_traits::element_type;
    using mask_type         = typename mask_traits::element_type;
    using data_type         = typename data_traits::element_type;
    
    static_assert(
        address_traits::length == 1 &&
        size_traits::length == 1 &&
        mask_traits::length == 1 &&
        data_traits::length == 1,
        "Unfortunately, the given HDL code uses a lot of bits (>= 65). "
        " The code is not written for such cases."
    );
    
    /**
     * @brief TL-UL Get operation.
     */
    struct get_op {
        std::function<void (std::vector<std::uint8_t> const &)> callback;
        address_type                                        address;
        size_type                                           size;
        mask_type                                           mask;
    };
    
    /**
     * @brief TL-UL PutFullData operation.
     */
    struct put_full_data_op {
        std::function<void ()>                              callback;
        address_type                                        address;
        size_type                                           size;
        mask_type                                           mask;
        std::vector<std::uint8_t>                           data;
    };
    
    /**
     * @brief TL-UL PutPartialData operation.
     */
    struct put_partial_data_op {
        std::function<void ()>                              callback;
        address_type                                        address;
        size_type                                           size;
        mask_type                                           mask;
        std::vector<std::uint8_t>                           data;
    };
    
    using op = boost::variant<get_op, put_full_data_op, put_partial_data_op>;
    
    /**
     * @brief Enqueues a Get operation. Please look at the get_op struct for the order and the
     * explanation of the parameters.
     * 
     * @param args Perfectly-forwarded to the constructor.
     */
    template <typename ...Args>
    void get(Args && ...args) {
        op_queue.push(get_op{std::forward<Args>(args)...});
    }
    
    /**
     * @brief Enqueues a PutFullData operation. Please look at the put_full_data_op struct for the
     * order and the explanation of the parameters.
     * 
     * @param args Perfectly-forwarded to the constructor.
     */
    template <typename ...Args>
    void put_full_data(Args && ...args) {
        op_queue.push(put_full_data_op{std::forward<Args>(args)...});
    }
    
    /**
     * @brief Enqueues a PutPartialData operation. Please look at the put_full_data_op struct for 
     * theorder and the explanation of the parameters.
     * 
     * @param args Perfectly-forwarded to the constructor.
     */
    template <typename ...Args>
    void put_partial_data(Args && ...args) {
        op_queue.push(put_partial_data_op{std::forward<Args>(args)...});
    }
    
    /**
     * @brief Wraps the eval of the managed HDL object.
     */
    void eval() {
        if (op_queue.empty()) {
            hdl->eval();
        }
        else {
            if (boost::apply_visitor(*this, op_queue.front()))
                op_queue.pop();
        }
    }
    
    
#define TLUL_TESTBENCH_ENSURE_OR_THROW(x) \
    if (!(x)) throw std::logic_error(std::string("Expected: ") + #x)
    
    /**
     * @brief executed when the current operation is a Get operation
     * @returns true if next operation is to be handled
     * @warning do not call by hand.
     */
    bool operator()(get_op &op) {
        bool next_state = false;
        
        // only for the rising edge
        if (hdl->CLK) {
            // READs here
            switch (opstate) {
                case opst_none: {
                    next_state = true;
                    break;
                }
                case opst_wrdy: {
                    if (hdl->a_ready) {
                        next_state = true;
                    }
                    break;
                }
                case opst_wack: {
                    if (hdl->d_valid) {
                        next_state = true;
                        
                        TLUL_TESTBENCH_ENSURE_OR_THROW(hdl->d_opcode == op_AccessAckData);
                        TLUL_TESTBENCH_ENSURE_OR_THROW(hdl->d_param == 0);
                        TLUL_TESTBENCH_ENSURE_OR_THROW(hdl->d_size == op.size);
                        TLUL_TESTBENCH_ENSURE_OR_THROW(hdl->d_source == 0);
                        // no restriction on d_sink
                        // we do not consider the error
                        
                        // we have a strong guarantee that hdl->d_data is POD
                        // make this case better by using unpacked arrays in SV
                        
                        // here, the mask is GUARANTEED to be contiguous
                        std::bitset<mask_traits::size_in_bits> bs(op.mask);
                        typename mask_traits::element_type mask_mask = 1;
                        
                        // each set bit in the mask corresponds to a byte
                        std::uintmax_t mask2 = 0xff; // select a byte
                        
                        std::vector<std::uint8_t> buf;
                        buf.reserve(1 << op.size);
                        
                        for (std::size_t i = 0; i < mask_traits::size_in_bits; ++i) {
                            if (op.mask & mask_mask) {
                                // if the mask is selected
                                // note: x << 3 is x * 8
                                buf.emplace_back((hdl->d_data & mask2) >> (i << 3));
                            }
                            mask_mask <<= 1;
                            mask2 <<= (1 << 3);
                        }
                        if (op.callback) {
                            op.callback(buf);
                        }
                    }
                    break;
                }
            }
        }
        
        hdl->eval();
        
        if (hdl->CLK) {
            // WRITEs here
            switch (opstate) {
                case opst_none: {
                    if (next_state) {
                        /* check if, */
                        // non zero bits in mask matches size
                        std::bitset<mask_traits::size_in_bits> bs{op.mask};
                        std::size_t sz = 1u << op.size;
                        TLUL_TESTBENCH_ENSURE_OR_THROW(bs.count() == sz);
                        
                        TLUL_TESTBENCH_ENSURE_OR_THROW(check_correct_mask(bs));
                        // what is natural alignment? IDK, not very well defined
                        // TLUL_TESTBENCH_ENSURE_OR_THROW(
                        //     check_contiguous(std::bitset<mask_traits::size_in_bits>(op.mask)));
                        
                        /* start writing */
                        hdl->a_valid = 1;
                        hdl->a_opcode = op_Get;
                        hdl->a_param = 0;
                        hdl->a_size = op.size;
                        hdl->a_source = 0;  // TODO temporarily
                        hdl->a_address = op.address;
                        hdl->a_mask = op.mask;
                        
                        // we have a strong guarantee that hdl->d_data is POD
                        // make this case better by using unpacked arrays in SV
                        // (not gonna repeat that anymore)
                        hdl->a_data = 0;
                        
                        /* get into the next state */
                        opstate = opst_wrdy;
                    }
                    break;
                }
                case opst_wrdy: {
                    if (next_state) {
                        hdl->a_valid = 0;
                        /* the rest of channel A is not important (don't care) */
                        
                        /* now, we are ready too. we can listen for an answer */
                        hdl->d_ready = 1;
                        
                        /* get into the next state */
                        opstate = opst_wack;
                    }
                    break;
                }
                case opst_wack: {
                    if (next_state) {
                        /* we are not ready yet */
                        hdl->d_ready = 0;
                        
                        opstate = opst_none;
                        
                        // we can now move to the next operation
                        return true;
                    }
                    break;
                }
            }
        }
        
        return false;
    }
    
    
    /**
     * @brief executed when the current operation is a Get operation
     * @returns true if next operation is to be handled
     * @warning do not call by hand.
     */
    bool operator()(put_full_data_op &op) {
        bool next_state = false;
        
        if (hdl->CLK) {
            switch (opstate) {
                case opst_none: {
                    next_state = true;
                    break;
                }
                case opst_wrdy: {
                    if (hdl->a_ready) {
                        next_state = true;
                    }
                    break;
                }
                case opst_wack: {
                    if (hdl->d_valid) {
                        next_state = true;
                        
                        TLUL_TESTBENCH_ENSURE_OR_THROW(hdl->d_opcode == op_AccessAck);
                        TLUL_TESTBENCH_ENSURE_OR_THROW(hdl->d_param == 0);
                        TLUL_TESTBENCH_ENSURE_OR_THROW(hdl->d_size == op.size);
                        TLUL_TESTBENCH_ENSURE_OR_THROW(hdl->d_source == 0);
                        // no restriction on d_sink
                        // no restriction on d_data
                        
                        // we do not consider the error case
                        
                        if (op.callback) {
                            op.callback();
                        }
                    }
                    break;
                }
            }
        }
        
        hdl->eval();
        
        if (hdl->CLK) {
            switch (opstate) {
                case opst_none: {
                    if (next_state) {
                        std::bitset<mask_traits::size_in_bits> bs{op.mask};
                        std::size_t sz = 1u << op.size;
                        TLUL_TESTBENCH_ENSURE_OR_THROW(op.data.size() == sz);
                        TLUL_TESTBENCH_ENSURE_OR_THROW(bs.count() == sz);
                        TLUL_TESTBENCH_ENSURE_OR_THROW(check_correct_mask(bs));
                        
                        hdl->a_valid = 1;
                        hdl->a_opcode = op_PutFullData;
                        hdl->a_param = 0;
                        hdl->a_size = op.size;
                        hdl->a_source = 0; // TODO temporarily
                        hdl->a_address = op.address;
                        hdl->a_mask = op.mask;
                        
                        // copy the payload, considering the MASK
                        {
                            auto it = op.data.begin();
                            for (auto i = 0; i < mask_traits::size_in_bits; ++i) {
                                if (get_bit(op.mask, i)) {
                                    hdl->a_data = set_byte(hdl->a_data, i, *it);
                                    ++it;
                                }
                            }
                        }
                        
                        opstate = opst_wrdy;
                    }
                    break;
                }
                case opst_wrdy: {
                    if (next_state) {
                        hdl->a_valid = 0;
                        hdl->d_ready = 1;
                        
                        opstate = opst_wack;
                    }
                    break;
                }
                case opst_wack: {
                    if (next_state) {
                        hdl->d_ready = 0;
                        
                        opstate = opst_none;
                        
                        return true;
                    }
                    break;
                }
            }
        }
        
        return false;
    }
    
    
    /**
     * @brief executed when the current operation is a Get operation
     * @returns true if next operation is to be handled
     * @warning do not call by hand.
     */
    bool operator()(put_partial_data_op &op) {
        return false;
    }
#undef TLUL_TESTBENCH_ENSURE_OR_THROW
private:
    // opcodes for several messages
    enum opcodes {
        op_Get                = 4,
        op_AccessAckData      = 1,
        op_PutFullData        = 0,
        op_PutPartialData     = 1,
        op_AccessAck          = 0
    };
    
    HDLSlaveMemory *hdl;
    
    // for operations in general
    enum opstate_enum {
        opst_none,
        opst_wrdy,
        opst_wack
    } opstate {opst_none};
    
    // for wait operation
    std::size_t left_cycles {0};
    
    std::queue<op> op_queue;
};

}

using detail::tlul_testbench;

#endif // TLUL_TESTBENCH_INCLUDED
