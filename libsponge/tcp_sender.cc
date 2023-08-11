#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , RTO(retx_timeout)
    ,_timer(0,false)
    , _stream(capacity) {}

//size_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    TCPSegment tcp_seg;
    if( _fin )
        return;
        
    if( !_syn ){
       tcp_seg.header().syn = true;
       _syn = true;
       send_TCPSegment(tcp_seg);
       return;
    }
    
    size_t window_size = (_receiver_window_size > 0 ? _receiver_window_size : 1);
    
    if( _stream.eof() && _abs_ackno + window_size > _next_seqno ){
        tcp_seg.header().fin = true;
        send_TCPSegment(tcp_seg);
        _fin = true;
        return;
    }
    
    while( !_stream.buffer_empty() && _abs_ackno + window_size > _next_seqno ){
        size_t length_send = min( TCPConfig::MAX_PAYLOAD_SIZE, window_size - _bytes_in_flight - tcp_seg.header().syn );
        tcp_seg.payload() = _stream.read(length_send);
        
        if( _stream.eof() &&  window_size - _bytes_in_flight > tcp_seg.length_in_sequence_space() ){
            tcp_seg.header().fin = true;
            //send_TCPSegment(tcp_seg);
            _fin = true;
        }
        send_TCPSegment(tcp_seg);
    }
    
}

void TCPSender::send_TCPSegment(TCPSegment &tcp_seg){
    tcp_seg.header().seqno = wrap( _next_seqno, _isn);
    
    _segments_out.push(tcp_seg);
    _segments_out_unacked.push(tcp_seg);
    
    _next_seqno += tcp_seg.length_in_sequence_space();
    _bytes_in_flight += tcp_seg.length_in_sequence_space();
    
    if( !_timer.timer_running_flag ){
        _timer.timer_running_flag = true;
        _timer.millisec = 0;
    }
    
    return;
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _receiver_window_size = window_size;
    uint64_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    if( abs_ackno > _next_seqno || abs_ackno <= _abs_ackno )
        return;
        
    _abs_ackno = abs_ackno;
    RTO = _initial_retransmission_timeout;
        
    bool pop = false;
    while( !_segments_out_unacked.empty() && _abs_ackno >= unwrap( _segments_out_unacked.front().header().seqno, _isn, _next_seqno) + _segments_out_unacked.front().length_in_sequence_space() ){
        _bytes_in_flight -= _segments_out_unacked.front().length_in_sequence_space();
        _segments_out_unacked.pop();
        
        pop = true;
    }
    
    if(pop){
        fill_window();
        _timer.millisec = 0;
        _consecutive_retransmission_times = 0;
    }
        
    _timer.timer_running_flag = _segments_out_unacked.empty() ? false:true;
    
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if( !_timer.timer_running_flag ){
        return;
    }
    
    _timer.millisec += ms_since_last_tick;
    if( _timer.millisec >= RTO && !_segments_out_unacked.empty() ){
        TCPSegment tcp_seg = _segments_out_unacked.front();
        _segments_out.push(tcp_seg);
        
        _timer.millisec = 0;
        _timer.timer_running_flag = true;
        
        if( _receiver_window_size > 0 ){
            RTO *= 2;
            _consecutive_retransmission_times++;
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmission_times; }

void TCPSender::send_empty_segment() {
    TCPSegment empty_seg;
    empty_seg.header().seqno = wrap( _next_seqno, _isn);
    _segments_out.push(empty_seg);
    return;
}
