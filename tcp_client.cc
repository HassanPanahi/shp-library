#include "tcp_client.h"
#include <iostream>
#include <boost/asio/detail/posix_thread.hpp>
#include "bounded_buffer.h"

namespace shp {
namespace network {

TCPClient::TCPClient(const TCPSocketShared& socket)
    : ip_(socket->remote_endpoint().address().to_string()), port_(socket->remote_endpoint().port()), socket_(socket)
{
    initialize();
    auto read_handler = boost::bind(&TCPClient::handle_read_data, this, boost::asio::placeholders::error ,boost::asio::placeholders::bytes_transferred);
    socket_->async_receive(boost::asio::buffer(data_.data(), data_.size()), read_handler);
    is_connected_ = true;
}

TCPClient::TCPClient(const std::string& ip, const unsigned short port)
    : ip_(ip), port_(port)
{
    is_connected_ = false;
    work_ = boost::make_shared<boost::asio::io_context::work>(io_context_);
    socket_ = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
    initialize();
}

void TCPClient::initialize()
{
    buffer_size_ = 1 * 1024;
    buffer_= std::make_shared<BoundedBuffer<uint8_t>>(buffer_size_);
    do_buffer_data_ = true;
    is_running_ = false;
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
            threads_group_.create_thread(boost::bind(&TCPClient::io_context_thread, this));
            auto read_handler = boost::bind(&TCPClient::handle_read_data, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
            socket_->async_receive(boost::asio::buffer(data_.data(), data_.size()), read_handler);
            is_connected_ = true;
        }
    }
    return is_connected_;
}

size_t TCPClient::send(const char *data, const uint32_t size)
{
    if (is_connected_ && size != 0 && data != nullptr)
        return socket_->send(boost::asio::buffer(data, size));
    return 0;
}

void TCPClient::async_send(const char *data, const uint32_t size, std::function<void (size_t)> func)
{
    if (is_connected_ && size != 0 && data != nullptr)
        socket_->async_send(boost::asio::buffer(data, size), boost::bind(func, boost::asio::placeholders::bytes_transferred));
}

void TCPClient::handle_read_data(const boost::system::error_code error, const size_t bytes_transferred)
{
    if (!error) {
        if (do_buffer_data_)
            buffer_->write(data_.data(), bytes_transferred);
        else
            data_received_connections_(reinterpret_cast<char*>(data_.data()), bytes_transferred);

        auto read_handler = boost::bind(&TCPClient::handle_read_data, this, boost::asio::placeholders::error ,boost::asio::placeholders::bytes_transferred);
        socket_->async_receive(boost::asio::buffer(data_.data(), data_.size()), read_handler);
    } else {
        if (is_connected_) {
            is_connected_ = false;
            disconnect_connections_();
            std::cout << "disconnect from server" << std::endl;
        }
    }
}

void TCPClient::do_buffer_data(const bool state)
{
    do_buffer_data_ = state;
}

boost::signals2::connection TCPClient::notify_me_when_disconnected(std::function<void ()>& func)
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

unsigned short TCPClient::get_port() const
{
    return port_;
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
    threads_group_.join_all();
    socket_.reset();
}

} // namespace peripheral
} // namespace hp

