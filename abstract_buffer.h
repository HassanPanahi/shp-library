#pragma once

#include <string>
#include <stdint.h>
#include <boost/call_traits.hpp>

enum class LibErros {
    OK,
    NOT_OK,
    Buffer_Overflow,
    Buffer_FULL,
    Buffer_Empty
};

template <typename T>
class AbstractBuffer
{
public:
    using param_type = typename std::decay<T>::type;

    virtual LibErros write(param_type item) = 0;
    virtual LibErros try_write(param_type item) = 0;
    virtual LibErros try_write_for(param_type item, const uint32_t duration_ms) = 0;


    virtual T read() = 0;
    virtual LibErros try_read(param_type& item) = 0;
    virtual LibErros try_read_for(param_type& item, const uint32_t duration_ms) = 0;

    virtual LibErros clear_buffer() = 0;
    virtual LibErros erase_buffer() = 0;
    virtual uint32_t get_remain() = 0;
    virtual ~AbstractBuffer() {}
};

