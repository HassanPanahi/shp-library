#include "tcp_client.h"
#include <iostream>
#include <boost/asio/detail/posix_thread.hpp>
#include "hp/common/buffer/circular_buffer.h"

namespace hp {
namespace peripheral {

std::atomic<int> TCPClient::ID_Counter_(0);

TCPClient::TCPClient(TCPSocketShared socket)
    : ip_(socket->remote_endpoint().address().to_string()), port_(socket->remote_endpoint().port()), socket_(socket)
{
    initialize();
    socket_->async_receive(boost::asio::buffer(data_.data(), data_.size()),
                           boost::bind(&TCPClient::handle_read_data, this, boost::asio::placeholders::error ,boost::asio::placeholders::bytes_transferred));
    is_connected_ = true;
}

TCPClient::TCPClient(std::string ip, short port)
    : ip_(ip), port_(port)
{
    is_connected_ = false;
    work_ = boost::make_shared<boost::asio::io_context::work>(io_context_);
    socket_ = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
    initialize();
}

void TCPClient::async_connect(std::function<void (int)> func)
{

}


void TCPClient::initialize()
{
    buffer_size_ = 1 * 1024;
    buffer_= std::make_shared<CircularBuffer>(buffer_size_);
    do_buffer_data_ = true;
    is_running_ = false;
    packet_structure_ = nullptr;
    buffer_is_mine_ = true;
    receive_size_ = 0;
    send_size_= 0;
    id_ = ID_Counter_;
    ID_Counter_++;
}

bool TCPClient::connect()
{
    if (!is_connected_) {
        boost::system::error_code ec;
        boost::asio::ip::tcp::resolver resolver(io_context_);
        boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), ip_, std::to_string(port_));
        endpoint_ = *resolver.resolve(query);
        auto error = socket_->connect(endpoint_, ec);
        if (error) {
            is_connected_ = false;
        } else {
            thread_group_.create_thread(boost::bind(&TCPClient::io_context_thread, this));
            socket_->async_receive(boost::asio::buffer(data_.data(), data_.size()),
                                   boost::bind(&TCPClient::handle_read_data, this, boost::asio::placeholders::error ,boost::asio::placeholders::bytes_transferred));
            is_connected_ = true;
        }
    }
    return is_connected_;
}


void TCPClient::start_find_messages(std::shared_ptr<AbstractPacketStructure> packet_structure)
{
    if (tcp_message_extractor_ != nullptr) {
        tcp_msg_extractor_lock_.lock();
        packet_structure_ = packet_structure;
        tcp_message_extractor_ = std::make_shared<MessageExtractor>(packet_structure_, buffer_);
        thread_group_.create_thread(boost::bind(&TCPClient::extract_message, this));
        tcp_msg_extractor_lock_.unlock();
    } else {
        tcp_msg_extractor_lock_.lock();
        tcp_message_extractor_ = std::make_shared<MessageExtractor>(packet_structure_, buffer_);
        tcp_msg_extractor_lock_.unlock();
    }
}

void TCPClient::extract_message()
{
    while(!is_connected_)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    while (is_connected_) {
        tcp_msg_extractor_lock_.lock();
        auto msg = tcp_message_extractor_->find_message();
        tcp_msg_extractor_lock_.unlock();
        if (msg == nullptr) {
            std::cout << "fucking null msg" << std::endl;
            exit(1);
        }
        messages_buffer_.write(msg);
    }
}

void TCPClient::set_buffer_size(size_t size_bytes)
{
    if (buffer_ != nullptr)
        buffer_size_ = size_bytes;
    else
        buffer_->set_new_buffer_size(size_bytes);
}

size_t TCPClient::send(const char *data, const uint32_t size)
{
    //TODO(HP) Do i must check these variables?
    if (is_connected_ /*&& size != 0 && data != nullptr*/) {
        send_size_ += size;
        return socket_->send(boost::asio::buffer(data, size));
    }
    return -1;
}

void TCPClient::async_send(const char *data, const uint32_t size, std::function<void (size_t)> func)
{
    if (is_connected_)
        socket_->async_send(boost::asio::buffer(data, size), boost::bind(func, boost::asio::placeholders::bytes_transferred));
}

void TCPClient::handle_read_data(const boost::system::error_code error, const size_t bytes_transferred)
{
    if (!error) {
        data_received_connections_((char*)data_.data(), bytes_transferred);
        if (do_buffer_data_)
            buffer_->write(data_.data(), bytes_transferred);
        receive_size_ += bytes_transferred;
        socket_->async_receive(boost::asio::buffer(data_.data(), data_.size()),
                               boost::bind(&TCPClient::handle_read_data, this, boost::asio::placeholders::error ,boost::asio::placeholders::bytes_transferred));
    } else {
        if (is_connected_) {
            is_connected_ = false;
            disconnect_connections_(id_);
            std::cout << "disconnect from server" << std::endl;
        }
    }
}

BufferError TCPClient::read_next_bytes(char *data, const uint32_t len, const uint32_t timeout_ms)
{
    if (buffer_ == nullptr || packet_structure_ != nullptr)
        return BufferError::BUF_NODATA;
    auto ret = buffer_->read(data, len, timeout_ms);
    return ret;
}

char TCPClient::read_next_byte()
{
    while(buffer_ == nullptr || packet_structure_ != nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    char data;
    char *data_ptr = &data;
    buffer_->read(data_ptr, 1);
    return data;
}

std::string TCPClient::read_all_bytes()
{
    if (buffer_ == nullptr || packet_structure_ != nullptr)
        return "";
    auto data = buffer_->get_all_bytes();
    return data;
}

uint32_t TCPClient::read_remain_bytes()
{
    if (buffer_ == nullptr || packet_structure_ != nullptr)
        return 0;
    uint32_t size = buffer_->get_remain_bytes();
    return size;
}

void TCPClient::do_buffer_data(const bool state)
{
    do_buffer_data_ = state;
}

void TCPClient::set_buffer(std::shared_ptr<AbstractBuffer> buffer)
{
    if (buffer_ == nullptr) {
        buffer_is_mine_ = false;
        buffer_ = buffer;
    } else {
        std::string all_data = buffer->get_all_bytes();
        buffer->write(all_data.data(), all_data.size());
        buffer_ = buffer;
    }
}


boost::signals2::connection TCPClient::notify_me_when_disconnected(std::function<void (int)> func)
{
    return disconnect_connections_.connect(func);
}

boost::signals2::connection TCPClient::notify_me_data_received(std::function<void (const char *, size_t)> func)
{
    //    is_buffered_data_ = false;
    return data_received_connections_.connect(func);
}

bool TCPClient::is_connected() const
{
    return is_connected_;
}

void TCPClient::disconnect()
{
    socket_->close();
    is_connected_ = false;
}

void TCPClient::io_context_thread()
{
    io_context_.run();
}

std::string TCPClient::get_ip() const
{
    return ip_;
}

short TCPClient::get_port() const
{
    return port_;
}

int TCPClient::get_client_id() const
{
    return id_;
}

std::shared_ptr<AbstractSerializableMessage> TCPClient::get_next_packet()
{
    return messages_buffer_.get_next_packet();
}

void TCPClient::print_send_receive_rate()
{
    static int sec = 0;
    if (send_size_ < 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " send " + std::to_string(send_size_) + " Bytes" << std::endl;
    } else if (send_size_ < 1024 * 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " send " + std::to_string((double)send_size_    / (1024) ) + " KB/s  " << std::endl;
    } else if (send_size_ < 1024 * 1024 * 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " send " + std::to_string((double)send_size_    / (1024 * 1024) ) + " MB/s  " << std::endl;
    } else if (send_size_ >= 1024 * 1024 * 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " send " + std::to_string((double)send_size_    / (1024*1024 * 1024) ) + " GB/s  " + std::to_string(send_size_) + " Bytes" << std::endl;
    }

    if (receive_size_ < 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " receive " + std::to_string(receive_size_) + " Bytes" << std::endl;
    } else if (receive_size_ < 1024 * 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " receive " + std::to_string((double)receive_size_ / (1024) ) + " KB/s  " << std::endl;
    } else if (receive_size_ < 1024 * 1024 * 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " receive " + std::to_string((double)receive_size_ / (1024 * 1024) ) + " MB/s  " << std::endl;
    } else if (receive_size_ >= 1024 * 1024 * 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " receive " + std::to_string((double)receive_size_ / (1024*1024 * 1024) ) + " GB/s  " + std::to_string(receive_size_) + " Bytes" << std::endl;
    }

    sec++;
    send_size_ = 0;
    receive_size_ = 0;
}

void TCPClient::print_send_rate()
{
    static int sec = 0;
    if (send_size_ < 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " send " + std::to_string(send_size_) + " Bytes" << std::endl;
    } else if (send_size_ < 1024 * 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " send " + std::to_string((double)send_size_    / (1024) ) + " KB/s  " << std::endl;
    } else if (send_size_ < 1024 * 1024 * 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " send " + std::to_string((double)send_size_    / (1024 * 1024) ) + " MB/s  " << std::endl;
    } else if (send_size_ >= 1024 * 1024 * 1024) {
        std::cout << "sec: " << sec << " ID: " << id_ << " send " + std::to_string((double)send_size_    / (1024*1024 * 1024) ) + " GB/s  " + std::to_string(send_size_) + " Bytes" << std::endl;
    }
    sec++;

    send_size_ = 0;
}

void TCPClient::print_receive_rate()
{
    if (is_connected_) {
        static int sec = 0;
        if (receive_size_ < 1024) {
            std::cout << "sec: " << sec << " ID: " << id_ << " receive " + std::to_string(receive_size_) + " Bytes" << std::endl;
        } else if (receive_size_ < 1024 * 1024) {
            std::cout << "sec: " << sec << " ID: " << id_ << " receive " + std::to_string((double)receive_size_ / (1024) ) + " KB/s  " << std::endl;
        } else if (receive_size_ < 1024 * 1024 * 1024) {
            std::cout << "sec: " << sec << " ID: " << id_ << " receive " + std::to_string((double)receive_size_ / (1024 * 1024) ) + " MB/s  " << std::endl;
        } else if (receive_size_ >= 1024 * 1024 * 1024) {
            std::cout << "sec: " << sec << " ID: " << id_ << " receive " + std::to_string((double)receive_size_ / (1024*1024 * 1024) ) + " GB/s  " + std::to_string(receive_size_) + " Bytes" << std::endl;
        }

        sec++;
        send_size_ = 0;
        receive_size_ = 0;
    }
}

TCPClient::~TCPClient()
{
    if (is_connected_) {
        socket_->close();
        is_connected_ = false;
    }
    disconnect();
    if (work_ != nullptr)
        work_.reset();
    thread_group_.join_all();
    socket_.reset();
    //    if (buffer_is_mine_)
    //        delete buffer_;
}

} // namespace peripheral
} // namespace hp

