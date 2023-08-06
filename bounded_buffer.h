#ifndef BOUNDEDBUFFER_H
#define BOUNDEDBUFFER_H

#include <mutex>
#include <functional>
#include <condition_variable>

#include <boost/circular_buffer.hpp>

#include "abstract_buffer.h"


/**
 *  @brief A buffer for record all data in circular buffer
 *
 *  @ingroup sequences
 *
 *  @tparam T  Type of element.
 *
*/

template <class T>
class BoundedBuffer : public AbstractBuffer<T> {
public:
    using container_type = boost::circular_buffer<T>;
    using size_type = typename container_type::size_type;
    using value_type = typename container_type::value_type;
    using param_type = typename AbstractBuffer<T>::param_type;

    explicit BoundedBuffer(const size_type capacity);
    BoundedBuffer(const BoundedBuffer& new_buffer);
    BoundedBuffer& operator=(const BoundedBuffer& new_buffer);

    virtual LibErros write(param_type item) override;
    virtual LibErros try_write(param_type item) override;
    virtual LibErros try_write_for(param_type item, const uint32_t duration_ms) override;

    virtual LibErros write(const param_type* const array, std::size_t size);

    template <typename Container>
    LibErros write(const Container& container);

    template <std::size_t N>
    LibErros write(const param_type (&array)[N]);

    virtual value_type read() override;
    virtual LibErros try_read(param_type& item) override;
    virtual LibErros try_read_for(param_type &item, const uint32_t duration_ms) ;

    bool empty() const;
    size_type size() const;
    size_type capacity() const;

    virtual LibErros clear_buffer() override;
    virtual LibErros erase_buffer() override;
    virtual uint32_t get_remain() override;

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
