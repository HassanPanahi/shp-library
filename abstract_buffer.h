#pragma once

#include <string>
#include <stdint.h>
#include <boost/call_traits.hpp>


enum BufferError {
    BUF_NOERROR = 0,
    BUF_OVERFLOW = 1,
    BUF_NODATA = 2,
    BUF_TIMEOUT = 3,
};

template <typename T>
class AbstractBuffer
{
public:
    using param_type = typename std::decay<T>::type;

    virtual BufferError read(param_type data, const size_t count, const uint32_t timeout_ms = 0) = 0;
    virtual BufferError write(param_type data, const size_t count) = 0;

    virtual BufferError clear_buffer() = 0;
    virtual BufferError erase_buffer() = 0;
    virtual uint32_t get_remain() = 0;
    virtual ~AbstractBuffer() {}
};

