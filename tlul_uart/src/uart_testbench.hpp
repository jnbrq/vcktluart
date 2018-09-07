/**
 * @author Canberk Sönmez
 * @file uart_testbench.hpp
 * @brief A UART wrapper for C++, which facilitates writing tests involving UART.
 * modelled after the given Verilog codes, the same constraints also apply here.
 */

#ifndef UART_TESTBENCH_HPP_INCLUDED
#define UART_TESTBENCH_HPP_INCLUDED

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <utility>
#include <functional>
#include <memory>
#include <utility>

#include "verilator_aux.hpp"

namespace uart {

template <typename CLK, typename TX>
struct sender {
    sender(CLK *clk, TX *tx, std::size_t cycles_per_bit):
        clk{clk},
        tx{tx},
        cycles_per_bit{cycles_per_bit} {
        if (clk == nullptr || tx == nullptr)
            throw std::runtime_error("uart::sender nullptr");
    }
    
    bool ongoing() const {
        return callback != nullptr;
    }
    
    template <typename Callback>
    bool write_byte(std::uint8_t byte, Callback &&callback) {
        if (ongoing())
            return false;
        this->byte = byte;
        this->callback = std::forward<Callback>(callback);
        if (!this->callback)
            return false;
        return true;
    }
    
    template <typename Callback>
    bool write_bytes(std::vector<std::uint8_t> data, Callback &&callback) {
        if (ongoing())
            return false;
        
        struct write_structure: std::enable_shared_from_this<write_structure> {
            sender *s;
            std::vector<std::uint8_t> data;
            std::function<void ()> callback;
            std::size_t idx{0};
            
            void start() {
                if (idx == data.size()) {
                    // write is complete
                    callback();
                    return ;
                }
                s->write_byte(data[idx], [this, _ = this->shared_from_this()] {
                    ++idx;
                    start();
                });
            }
        };
        
        auto w = std::make_shared<write_structure>();
        w->s = this;
        w->data = std::move(data);
        w->callback = std::move(callback);
        w->idx = 0;
        w->start();
    }
    
    template <typename Callback>
    bool write_bytes(std::string const &str, Callback &&callback) {
        return write_bytes(std::vector<std::uint8_t>(
            str.begin(), str.end()), std::forward<Callback>(callback));
    }
    
    // must be called after the main model
    void eval() {
        using namespace verilator_aux;
        
        if (*clk) {
            switch (r_SM_Main) {
                case s_IDLE: {
                    *tx = 1;
                    r_Clock_Count = 0;
                    r_Bit_Index = 0;
                    
                    if (callback /* write requested */) {
                        r_SM_Main = s_TX_START_BIT;
                    }
                    break;
                }
                case s_TX_START_BIT: {
                    *tx = 0;
                    if (r_Clock_Count < cycles_per_bit - 1) {
                        ++r_Clock_Count;
                    }
                    else {
                        r_Clock_Count = 0;
                        r_SM_Main = s_TX_DATA_BITS;
                    }
                    break;
                }
                case s_TX_DATA_BITS: {
                    *tx = get_bit(byte, r_Bit_Index);
                    
                    if (r_Clock_Count < cycles_per_bit - 1) {
                        ++r_Clock_Count;
                    }
                    else {
                        r_Clock_Count = 0;
                        if (r_Bit_Index < 7) {
                            r_Bit_Index = r_Bit_Index + 1;
                        }
                        else {
                            r_Bit_Index = 0;
                            r_SM_Main = s_TX_STOP_BIT;
                        }
                    }
                    break;
                }
                case s_TX_STOP_BIT: {
                    *tx = 1;
                    if (r_Clock_Count < cycles_per_bit - 1) {
                        ++r_Clock_Count;
                    }
                    else {
                        r_Clock_Count = 0;
                        r_SM_Main = s_CLEANUP;
                    }
                    break;
                }
                case s_CLEANUP: {
                    auto cb = std::move(callback);
                    callback = nullptr;
                    cb();   // to allow setting from callback function, callback = nullptr avoided
                    r_SM_Main = s_IDLE;
                    break;
                }
                default: {
                    r_SM_Main = s_IDLE;
                    break;
                }
            }
        }
    }
private:
    std::uint8_t byte;
    std::function<void ()> callback;
    
    CLK *clk;
    TX *tx;
    std::size_t cycles_per_bit;
    
    enum {
        s_IDLE,
        s_TX_START_BIT,
        s_TX_DATA_BITS,
        s_TX_STOP_BIT,
        s_CLEANUP
    } r_SM_Main {s_IDLE};
    
    std::size_t r_Clock_Count {0};
    std::uint8_t r_Bit_Index {0};
};

template <typename CLK, typename RX>
struct receiver {
    template <typename Callback>
    receiver(Callback &&callback, CLK *clk, RX *rx, std::size_t cycles_per_bit):
        callback{std::forward<Callback>(callback)},
        clk{clk},
        rx{rx},
        cycles_per_bit{cycles_per_bit} {
        if (clk == nullptr || rx == nullptr)
            throw std::runtime_error("uart::sender nullptr");
    }
    
    // must be called before the main model
    void eval() {
        using namespace verilator_aux;
        
        if (*clk) {
            switch (r_SM_Main) {
                case s_IDLE: {
                    r_Clock_Count = 0;
                    r_Bit_Index = 0;
                    
                    if (*rx == 0) {
                        r_SM_Main = s_RX_START_BIT;
                    }
                    break;
                }
                case s_RX_START_BIT: {
                    if (r_Clock_Count == (cycles_per_bit-1)/2) {
                        if (*rx == 0) {
                            r_Clock_Count = 0;
                            r_SM_Main = s_RX_DATA_BITS;
                        }
                        else {
                            r_SM_Main = s_IDLE;
                        }
                    }
                    else {
                        ++r_Clock_Count;
                    }
                    break;
                }
                case s_RX_DATA_BITS: {
                    if (r_Clock_Count < cycles_per_bit-1) {
                        ++r_Clock_Count;
                    }
                    else {
                        r_Clock_Count = 0;
                        byte = set_bit(byte, r_Bit_Index, *rx);
                        if (r_Bit_Index < 7) {
                            ++r_Bit_Index;
                        }
                        else {
                            r_Bit_Index = 0;
                            r_SM_Main = s_RX_STOP_BIT;
                        }
                    }
                    break;
                }
                case s_RX_STOP_BIT: {
                    if (r_Clock_Count < cycles_per_bit - 1) {
                        ++r_Clock_Count;
                    }
                    else {
                        r_Clock_Count = 0;
                        r_SM_Main = s_CLEANUP;
                    }
                    break;
                }
                case s_CLEANUP: {
                    if (callback)
                        callback(byte);
                    r_SM_Main = s_IDLE;
                }
                default: {
                    r_SM_Main = s_IDLE;
                    break;
                }
            }
        }
    }
private:
    std::function<void (std::uint8_t)> callback;
    CLK *clk;
    RX *rx;
    std::size_t cycles_per_bit;
    
    std::uint8_t byte;
    
    enum {
        s_IDLE,
        s_RX_START_BIT,
        s_RX_DATA_BITS,
        s_RX_STOP_BIT,
        s_CLEANUP
    } r_SM_Main {s_IDLE};
    
    std::size_t r_Clock_Count {0};
    std::uint8_t r_Bit_Index {0};
};

template <typename CLK, typename TX>
auto make_sender(CLK *clk, TX *tx, std::size_t cycles_per_bit) {
    return sender<CLK, TX>{clk, tx, cycles_per_bit};
}

template <typename Callback, typename CLK, typename RX>
auto make_receiver(Callback &&callback, CLK *clk, RX *rx, std::size_t cycles_per_bit) {
    return receiver<CLK, RX>{std::forward<Callback>(callback), clk, rx, cycles_per_bit};
}

};

#endif // UART_TESTBENCH_HPP_INCLUDED
