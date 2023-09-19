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
    using container = boost::circular_buffer<T>;
    using size_type = typename container::size_type;
    using value_type = typename container::value_type;
    using param_type = typename AbstractBuffer<T>::param_type;
    using AbstractBuffer<T>::write;

    explicit BoundedBuffer(const size_type capacity);
    BoundedBuffer(const BoundedBuffer& new_buffer);
    BoundedBuffer& operator=(const BoundedBuffer& new_buffer);

    virtual LibErros write(param_type item) override;
    virtual LibErros try_write(param_type item) override;
    virtual LibErros try_write_for(param_type item, const uint32_t duration_ms) override;

    virtual value_type read() override;
    virtual LibErros try_read(param_type& item) override;
    virtual LibErros try_read_for(param_type &item, const uint32_t duration_ms) override;

    bool empty() const;
    size_type size() const;
    size_type capacity() const;

    virtual LibErros clear_buffer() override;
    virtual LibErros erase_buffer() override;
    virtual uint32_t get_remain() override;

private:
    bool is_not_full() const;
    bool is_not_empty() const;
    container container_;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
};

#include "bounded_buffer.cc"

#endif // BOUNDEDBUFFER_H
