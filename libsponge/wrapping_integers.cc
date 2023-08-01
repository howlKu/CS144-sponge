#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint32_t temp = isn.raw_value() + static_cast<uint32_t>(n);
        
    return WrappingInt32{temp};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t temp = 0;
    if( n - isn < 0 ){
        temp = static_cast<uint64_t>( n - isn + (1l<<32));
    }else{
        temp = static_cast<uint64_t>(n - isn);
    }
    if( temp >= checkpoint ){
        return temp;
    }
    temp |= (checkpoint >> 32) << 32;
    if( temp < checkpoint ){
        temp += (1ll<<32);
    }
    return temp - checkpoint < checkpoint + (1ll<<32) - temp? temp : temp - (1ll<<32);
    
}
