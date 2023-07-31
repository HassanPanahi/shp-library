#ifndef BOUNDEDBUFFER_H
#define BOUNDEDBUFFER_H

#include <boost/circular_buffer.hpp>
#include <boost/call_traits.hpp>
#include <condition_variable>
#include <functional>
#include "mutex"

enum class LibErros {
    OK,
    NOT_OK,
    Buffer_Overflow,
    Buffer_FULL,
    Buffer_Empty
};

/**
 *  @brief A buffer for record all data in circular buffer
 *
 *  @ingroup sequences
 *
 *  @tparam T  Type of element.
 *
 *
 *
*/

template <class T>
class BoundedBuffer {
public:
    using container_type = boost::circular_buffer<T>;
    using size_type = typename container_type::size_type;
    using value_type = typename container_type::value_type;
    using param_type = typename std::decay<value_type>::type;

    explicit BoundedBuffer(const size_type capacity);
    BoundedBuffer(const BoundedBuffer& new_buffer);
    BoundedBuffer& operator=(const BoundedBuffer& new_buffer);

    LibErros write(const param_type& item);
    LibErros try_write(param_type item);
    LibErros try_write_for(param_type item, const uint32_t duration_ms);

    template <typename Container>
    LibErros write(const Container& container);

    LibErros write(const value_type* array, std::size_t size);

    template <std::size_t N>
    LibErros write(const value_type (&array)[N]);

    value_type read();
    LibErros try_read(value_type& item);
    LibErros try_read_for(value_type& item, const uint32_t duration_ms);

    bool empty() const;
    size_type size() const;
    size_type capacity() const;

private:
    bool is_not_full() const;
    bool is_not_empty() const;
    container_type m_container;
    std::mutex m_mutex;
    std::condition_variable m_not_empty;
    std::condition_variable m_not_full;
};

#include "bounded_buffer.cc"

#endif // BOUNDEDBUFFER_H
