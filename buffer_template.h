#ifndef BUFFER_H
#define BUFFER_H

#include "abstract_buffer.h"

#include <vector>
#include <string>
#include <atomic>
#include <thread>

namespace hp {
namespace peripheral {

template <class Type>
class Buffer
{
public:
    Buffer(uint32_t size = 200);
    void write(Type data);
    Type get_next_packet();
    ~Buffer();
private:
    uint32_t size_;
    std::atomic<uint32_t> write_index_;
    std::atomic<uint32_t> read_index_;
    std::vector<Type> buffer_;
};


template <class Type>
Buffer<Type>::Buffer(uint32_t size)
    : size_(size)
{
    write_index_ = 0;
    read_index_ = 0;
    buffer_.resize(size_);
}

template<class Type>
void Buffer<Type>::write(Type data)
{
    buffer_[write_index_] = data;
    write_index_++;
    if (write_index_ == size_)
        write_index_ = 0;
}

template<class Type>
Type Buffer<Type>::get_next_packet()
{
    while (read_index_ == write_index_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    Type msg = buffer_[read_index_];
    read_index_++;
    if (read_index_ == size_)
        read_index_ = 0;
    return msg;
}

template<class Type>
Buffer<Type>::~Buffer()
{

}


} // namespace peripheral
} // namespace hp

#endif // BUFFER_H
