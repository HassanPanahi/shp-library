#include "bounded_buffer.h"

template <class T>
BoundedBuffer<T>::BoundedBuffer(const size_type capacity) : container_(capacity)
{

}

template <class T>
BoundedBuffer<T>::BoundedBuffer(const BoundedBuffer& new_buffer) {
    std::unique_lock<std::mutex> lock_this(mutex_);
    container_ = new_buffer.container_;
}

template <class T>
BoundedBuffer<T>& BoundedBuffer<T>::operator=(const BoundedBuffer& new_buffer) {
    if (this != &new_buffer) {
        std::unique_lock<std::mutex> lock_this(mutex_);
        container_ = new_buffer.container_;
    }
    return *this;
}


template <class T>
LibErros BoundedBuffer<T>::write(param_type item) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_.wait(lock, [this] { return is_not_full(); });
    container_.push_front(item);
    lock.unlock();
    not_empty_.notify_one();
    return LibErros::OK;
}

template <class T>
LibErros BoundedBuffer<T>::try_write(param_type item) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto ret = LibErros::OK;
    if (!is_not_full()) {
        ret = LibErros::Buffer_FULL;
    } else {
        container_.push_front(item);
        lock.unlock();
        not_empty_.notify_one();
    }
    return ret;
}

template <class T>
LibErros BoundedBuffer<T>::try_write_for(param_type item, const uint32_t duration_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto ret = LibErros::OK;
    bool is_ok = not_full_.wait_for(lock, std::chrono::milliseconds(duration_ms), [this] { return is_not_full(); });
    if (!is_ok) {
        ret = LibErros::Buffer_FULL;
    } else {
        container_.push_front(item);
        lock.unlock();
        not_empty_.notify_one();
    }
    return ret;
}

template <class T>
typename BoundedBuffer<T>::value_type BoundedBuffer<T>::read() {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, [this] { return is_not_empty(); });
    value_type result = container_.back();
    container_.pop_back();
    lock.unlock();
    not_full_.notify_one();
    return result;
}

template <class T>
LibErros BoundedBuffer<T>::try_read(param_type &item) {
    auto ret = LibErros::OK;
    std::unique_lock<std::mutex> lock(mutex_);
    if (!is_not_empty()) {
        ret = LibErros::Buffer_Empty;
    } else {
        item = container_.back();
        container_.pop_back();
        not_full_.notify_one();
    }
    return ret;
}

template <class T>
LibErros BoundedBuffer<T>::try_read_for(param_type& item, const uint32_t duration_ms) {
    auto ret = LibErros::OK;
    std::unique_lock<std::mutex> lock(mutex_);
    bool is_ok = not_empty_.wait_for(lock, std::chrono::milliseconds(duration_ms), [this] { return is_not_empty(); });
    if (is_ok) {
        ret = LibErros::Buffer_Empty;
    } else {
        item = container_.back();
        container_.pop_back();
        not_full_.notify_one();
    }
    return ret;
}

template <class T>
bool BoundedBuffer<T>::empty() const {
    return container_.empty();
}

template <class T>
typename BoundedBuffer<T>::size_type BoundedBuffer<T>::size() const {
    return container_.size();
}

template <class T>
typename BoundedBuffer<T>::size_type BoundedBuffer<T>::capacity() const {
    return container_.capacity();
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
    return capacity() - size();
}

template <class T>
bool BoundedBuffer<T>::is_not_full() const {
    return !container_.full();
}

template <class T>
bool BoundedBuffer<T>::is_not_empty() const {
    return !empty();
}
