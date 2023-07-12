#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capa): byte_stream(""), capacity(capa), written_bytes(0), read_bytes(0), end_write(false), end_read(false) { }

size_t ByteStream::write(const string &data) {
    if( data.length() <= remaining_capacity() ){
    	byte_stream += data;
	written_bytes += data.length();
	return data.length();
    }else{
	written_bytes += remaining_capacity();
	size_t len_real = remaining_capacity();
    	byte_stream += data.substr( 0, len_real );
	return len_real;
    }

}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t len_real = min( len, byte_stream.length() );
    return byte_stream.substr( 0, len_real);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t len_real = min( len, byte_stream.length() );
    byte_stream.erase( 0, len_real);
    read_bytes += len_real;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string str_temp = peek_output(len);
    pop_output(len);
    return str_temp;
}

void ByteStream::end_input() {	end_write = true; }

bool ByteStream::input_ended() const { return end_write; }

size_t ByteStream::buffer_size() const { return byte_stream.length(); }

bool ByteStream::buffer_empty() const { return byte_stream.empty(); }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return written_bytes; }

size_t ByteStream::bytes_read() const { return read_bytes; }

size_t ByteStream::remaining_capacity() const { return capacity - byte_stream.length(); }
