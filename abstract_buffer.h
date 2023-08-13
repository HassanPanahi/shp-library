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

    template <typename Container>
    LibErros write(const Container& container) {
        static_assert(std::is_same<typename Container::value_type, param_type>::value,
                      "Container value_type must match the value_type of BoundedBuffer");
        auto ret = LibErros::OK;
        for (const auto& item : container) {
            ret = write(item);
            if (ret != LibErros::OK)
                break;
        }
        return ret;
    }

    template <std::size_t N>
    LibErros write(const param_type (&array)[N])
    {
        auto ret = write(array, N);
        return ret;
    }

    LibErros write(const param_type * const array, std::size_t size)
    {
        auto ret = LibErros::OK;
        for (std::size_t i = 0; i < size; ++i) {
            ret = write(array[i]);
            if (ret != LibErros::OK)
                break;
        }
        return ret;
    }



    virtual T read() = 0;
    virtual LibErros try_read(param_type& item) = 0;
    virtual LibErros try_read_for(param_type& item, const uint32_t duration_ms) = 0;

    virtual LibErros read(param_type * const array, std::size_t size)
    {
        auto ret = LibErros::OK;
        for (std::size_t i = 0; i < size; ++i)
            array[i] = read();
        return ret;
    }

    virtual LibErros clear_buffer() = 0;
    virtual LibErros erase_buffer() = 0;
    virtual uint32_t get_remain() = 0;
    virtual ~AbstractBuffer() {}
};

