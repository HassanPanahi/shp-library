#include "bounded_buffer.h"

template <class T>
BoundedBuffer<T>::BoundedBuffer(const size_type capacity) : m_container(capacity)
{

}

template <class T>
BoundedBuffer<T>::BoundedBuffer(const BoundedBuffer& new_buffer) {
    std::unique_lock<std::mutex> lock_this(m_mutex);
    m_container = new_buffer.m_container;
}

template <class T>
BoundedBuffer<T>& BoundedBuffer<T>::operator=(const BoundedBuffer& new_buffer) {
    if (this != &new_buffer) {
        std::unique_lock<std::mutex> lock_this(m_mutex);
        m_container = new_buffer.m_container;
    }
    return *this;
}


template <class T>
LibErros BoundedBuffer<T>::write(param_type item) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_not_full.wait(lock, [this] { return is_not_full(); });
    m_container.push_front(item);
    lock.unlock();
    m_not_empty.notify_one();
    return LibErros::OK;
}

template <class T>
LibErros BoundedBuffer<T>::try_write(param_type item) {
    std::unique_lock<std::mutex> lock(m_mutex);
    auto ret = LibErros::OK;
    if (!is_not_full()) {
        ret = LibErros::Buffer_FULL;
    } else {
        m_container.push_front(item);
        lock.unlock();
        m_not_empty.notify_one();
    }
    return ret;
}

template <class T>
LibErros BoundedBuffer<T>::try_write_for(param_type item, const uint32_t duration_ms) {
    std::unique_lock<std::mutex> lock(m_mutex);
    auto ret = LibErros::OK;
    bool is_ok = m_not_full.wait_for(lock, std::chrono::milliseconds(duration_ms), [this] { return is_not_full(); });
    if (!is_ok) {
        ret = LibErros::Buffer_FULL;
    } else {
        m_container.push_front(item);
        lock.unlock();
        m_not_empty.notify_one();
    }
    return ret;
}

template<class T>
LibErros BoundedBuffer<T>::write(const param_type * const array, std::size_t size)
{
    auto ret = LibErros::OK;
    for (std::size_t i = 0; i < size; ++i) {
        ret = write(array[i]);
        if (ret != LibErros::OK)
            break;
    }
    return ret;
}

template <class T>
template <typename Container>
LibErros BoundedBuffer<T>::write(const Container& container) {
    static_assert(std::is_same<typename Container::value_type, value_type>::value,
                  "Container value_type must match the value_type of BoundedBuffer");
    auto ret = LibErros::OK;
    for (const auto& item : container) {
        ret = write(item);
        if (ret != LibErros::OK)
            break;
    }
    return ret;
}

template <class T>
template <std::size_t N>
LibErros BoundedBuffer<T>::write(const param_type (&array)[N])
{
    auto ret = write(array, N);
    return ret;
}

template <class T>
typename BoundedBuffer<T>::value_type BoundedBuffer<T>::read() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_not_empty.wait(lock, [this] { return is_not_empty(); });
    value_type result = m_container.back();
    m_container.pop_back();
    lock.unlock();
    m_not_full.notify_one();
    return result;
}

template <class T>
LibErros BoundedBuffer<T>::try_read(param_type &item) {
    auto ret = LibErros::OK;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!is_not_empty()) {
        ret = LibErros::Buffer_Empty;
    } else {
        item = m_container.back();
        m_container.pop_back();
        m_not_full.notify_one();
    }
    return ret;
}

template <class T>
LibErros BoundedBuffer<T>::try_read_for(param_type& item, const uint32_t duration_ms) {
    auto ret = LibErros::OK;
    std::unique_lock<std::mutex> lock(m_mutex);
    bool is_ok = m_not_empty.wait_for(lock, std::chrono::milliseconds(duration_ms), [this] { return is_not_empty(); });
    if (is_ok) {
        ret = LibErros::Buffer_Empty;
    } else {
        item = m_container.back();
        m_container.pop_back();
        m_not_full.notify_one();
    }
    return ret;
}

template <class T>
bool BoundedBuffer<T>::empty() const {
    return m_container.empty();
}

template <class T>
typename BoundedBuffer<T>::size_type BoundedBuffer<T>::size() const {
    return m_container.size();
}

template <class T>
typename BoundedBuffer<T>::size_type BoundedBuffer<T>::capacity() const {
    return m_container.capacity();
}

template<class T>
LibErros BoundedBuffer<T>::clear_buffer()
{
    return LibErros::Buffer_Empty;

}

template<class T>
LibErros BoundedBuffer<T>::erase_buffer()
{
    return LibErros::Buffer_Empty;

}

template<class T>
uint32_t BoundedBuffer<T>::get_remain()
{
    return 0;

}

template <class T>
bool BoundedBuffer<T>::is_not_full() const {
    return !m_container.full();
}

template <class T>
bool BoundedBuffer<T>::is_not_empty() const {
    return !empty();
}
