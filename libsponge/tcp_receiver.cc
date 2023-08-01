#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &header = seg.header();
    if( header.syn ){
        _isn = header.seqno;
        _syn_set_flag = true;
    }
    
    if( !_syn_set_flag )
        return;
    
    //uint64_t abs_seqno = unwrap( header.seqno, _isn, stream_out().bytes_written());
    //if( header.syn )
    //    abs_seqno++;
    
    uint64_t abs_seqno = unwrap(header.seqno, _isn, stream_out().bytes_written());
    uint64_t stream_index = abs_seqno - 1 + (header.syn);
        
    _reassembler.push_substring( seg.payload().copy(), stream_index, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if( !_syn_set_flag )
        return nullopt;
    
    uint64_t abs_ackno = stream_out().bytes_written() + 1;
    if( stream_out().input_ended() )
        abs_ackno++;
    
    return wrap( abs_ackno , _isn);
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
