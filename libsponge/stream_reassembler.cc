#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) 
    : _unassembled_strs(),
      _next_assembled_idx(0),
      _unassembled_bytes_num(0),
      _eof_idx(-1),
      _output(capacity),
      _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    
    if( index + data.length() <= _next_assembled_idx ){
    }else if( index <= _next_assembled_idx && index + data.length() > _next_assembled_idx ){
        string new_data = data.substr( _next_assembled_idx - index);
        size_t new_index = _next_assembled_idx;
        size_t bytes_written = _output.write( new_data);
        if( bytes_written < new_data.length() ){
            new_index += bytes_written;
            _push_temporary_storage( new_data.substr(bytes_written), new_index);
        }
        _next_assembled_idx += bytes_written;
    }else{
        _push_temporary_storage( data, index);
    }
    
    for( map<size_t,string>::iterator it = _unassembled_strs.begin(); it != _unassembled_strs.end(); ){
        if( it->first > _next_assembled_idx ){
            ++it;
            continue;
        }else if( it->first + it->second.length() <= _next_assembled_idx ){
            _unassembled_bytes_num -= it->second.length();
            it = _unassembled_strs.erase(it);
            continue;
        }else{
            size_t it_index = it->first;
            string it_data = it->second;
            _unassembled_bytes_num -= it_data.length();
            it_data.erase( 0, _next_assembled_idx - it_index);
            it_index = _next_assembled_idx;
            size_t bytes_written = _output.write( it_data);
            if( bytes_written < it_data.length() ){
                it_index += bytes_written;
                //_unassembled_bytes_num -= it_data.length();
                it = _unassembled_strs.erase(it);
                _push_temporary_storage( it_data.substr( bytes_written), it_index);
                _next_assembled_idx += bytes_written;
                
                continue;
            }
            _next_assembled_idx += bytes_written;
            //_unassembled_bytes_num -= bytes_written;
            it = _unassembled_strs.erase(it);
        }
           
        
    }
    
    if (eof)
        _eof_idx = index + data.length();
    if (_eof_idx <= _next_assembled_idx)
        _output.end_input();
}

size_t StreamReassembler::_push_temporary_storage(const string &data, const size_t index){
    size_t len_pushed = 0;
    string new_data = data;
    size_t new_index = index;
    
    auto it_f = _unassembled_strs.upper_bound(index);
    if( it_f != _unassembled_strs.begin() ){
        it_f--;
        if( index >= it_f->first + it_f->second.length() ){
        }else if( index + data.length() <= it_f->first + it_f->second.length() ){
            return 0;
        }else{
            new_data.erase(0, it_f->first + it_f->second.length() - index );
            new_index = it_f->first + it_f->second.length();
            if( new_data.empty() )
                return 0;
        }
    }
    
    auto it_b = _unassembled_strs.lower_bound(new_index);
    if( it_b != _unassembled_strs.end() ){
        if( new_index + new_data.length() <= it_b->first ){
        }else if( new_index + new_data.length() >= it_b->first + it_b->second.length() ){
            _unassembled_bytes_num -= it_b->second.length();
            it_b = _unassembled_strs.erase(it_b);
            while( it_b != _unassembled_strs.end() ){
                if( new_index + new_data.length() <= it_b->first ){
                    break;
                }else if( new_index + new_data.length() >= it_b->first + it_b->second.length() ){
                    _unassembled_bytes_num -= it_b->second.length();
                    it_b = _unassembled_strs.erase(it_b);
                }else{
                    new_data = new_data.substr(0, it_b->first - new_index);
                    if( new_data.empty() )
                        return 0;
                    break;
                }
            }
            
        }else{
            new_data = new_data.substr(0, it_b->first - new_index);
            if( new_data.empty() )
                return 0;
        }
    }
    
    
    len_pushed = min( new_data.length(), _unassembled_strs_remaining_capacity() );
    _unassembled_bytes_num += len_pushed;
    _unassembled_strs[new_index] = new_data.substr(0, len_pushed);
    //_unassembled_bytes_num += new_data.length();
    //_unassembled_strs[new_index] = new_data;
        
    
    return len_pushed;
}

size_t StreamReassembler::unassembled_bytes() const{ 
    return _unassembled_bytes_num; 
    //size_t sum = 0;
    //for( auto it = _unassembled_strs.begin(); it != _unassembled_strs.end(); ++it){
    //    sum += it->second.length();
    //}
    //return sum;
}

bool StreamReassembler::empty() const { return _unassembled_strs.empty(); }

size_t StreamReassembler::_unassembled_strs_remaining_capacity() const { 
    return _capacity - _output.buffer_size() - _unassembled_bytes_num; 
    //return _capacity - _unassembled_bytes_num;
}
