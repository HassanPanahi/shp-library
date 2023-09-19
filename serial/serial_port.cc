#include "serial_port.h"
#include <iostream>
#include <dirent.h>

#include "../buffer/bounded_buffer.h"

namespace hp {
namespace peripheral {

SerialPort::SerialPort(void)
{
    is_set_buffer_ = false;
    buffer_size_ = 2 * 1024 * 1024;
//    buffer_ = nullptr;
    is_set_extractor_ = false;
    is_buffered_data_ = true;
    is_running_ = false;
}

SerialPort::~SerialPort(void)
{
    thread_group_.join_all();
    stop();
}

std::vector<std::string> SerialPort::get_port_names()
{
    std::vector<std::string> names;
    DIR* pdir = opendir("/dev");
    struct dirent *pent;
    if (pdir) {
        while ((pent = readdir(pdir))) {
            if ( (strstr(pent->d_name, "ttyS") != 0) ||
                 (strstr(pent->d_name, "ttyUSB") != 0)  ||
                 (strstr(pent->d_name, "ttyACM") != 0) ||
                 (strstr(pent->d_name, "ttymxc") != 0))
            {
                std::string p = ("/dev/");
                p.append(pent->d_name);
                if (is_port_accessible(p))
                    names.push_back(p.c_str());
            }
        }
    }
    return names;
}


void SerialPort::print_ports()
{
    auto ports = get_port_names();
    for (const auto &port : ports)
        std::cout << port << std::endl;
}

bool SerialPort::is_port_accessible(std::string& port)
{
    try
    {
        boost::asio::io_service service;
        boost::asio::serial_port sp(service, port);

        if (sp.is_open()) {
            sp.close();
            return true;
        }
    }
    catch( std::exception& )
    {
        return false;
    }

    return false;

}

void SerialPort::print_send_receive_rate()
{

}

void SerialPort::print_send_rate()
{

}

void SerialPort::print_receive_rate()
{

}

void SerialPort::extract_message()
{
//    serial_message_extractor_ = boost::make_shared<MessageExtractor>(msg_extractor_, buffer_);
//    while(is_running_) {
//        auto msg = serial_message_extractor_->find_message();
//        if (msg == nullptr) {
//            std::cout << "fucking null msg" << std::endl;
//            exit(1);
//        }
//        messages_buffer_.write(msg);
//    }
}

bool SerialPort::start(const std::string& port, SerialBaudRate baud_rate)
{
    boost::system::error_code ec;

    if (port_) {
        std::cout << "error : port is already opened..." << std::endl;
        return false;
    }

    port_ = boost::make_shared<boost::asio::serial_port>(io_service_);

    port_->set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    port_->set_option(boost::asio::serial_port_base::character_size(8));
    port_->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    port_->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    port_->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
    port_->open(port.data(), ec);
    if (ec) {
        std::cout << "error : port_->open() failed...com_port_name=" << port << ", e=" << ec.message().c_str() << std::endl;
        return false;
    }

    is_running_  = true;
//    if (!is_set_buffer_)
//        buffer_ = std::make_shared<CircularBuffer>(buffer_size_);
    if (is_set_extractor_) {
        thread_group_.create_thread(boost::bind(&SerialPort::extract_message, this));
    }
    thread_group_.create_thread(boost::bind(&boost::asio::io_service::run, &io_service_));

    async_read_some();

    return true;
}

void SerialPort::stop()
{
    boost::mutex::scoped_lock look(mutex_);

    if (port_) {
        port_->cancel();
        port_->close();
        port_.reset();
    }
    io_service_.stop();
    io_service_.reset();
}

//void SerialPort::set_buffer(std::shared_ptr<AbstractBuffer> buffer)
//{
//    is_set_buffer_ = true;
//    buffer_ = buffer;
//}

//void SerialPort::set_buffer_size(uint32_t size)
//{
//    buffer_size_ = size;
//}

//void SerialPort::extract_messages(std::shared_ptr<AbstractPacketStructure> extractor)
//{
//    serial_message_extractor_ = extractor;
//    is_set_extractor_ = true;
//}

//std::shared_ptr<AbstractSerializableMessage> SerialPort::get_next_packet()
//{
//    return messages_buffer_.get_next_packet();
//}

//BufferError SerialPort::get_next_bytes(uint8_t *data, const uint32_t len, const uint32_t timeout_ms)
//{
//    return buffer_->read(data, len, timeout_ms);
//}

//uint8_t SerialPort::get_next_byte()
//{
//    return buffer_->read_next_byte();

//}
//std::string SerialPort::get_all_bytes()
//{
//    return buffer_->get_all_bytes();
//}

//uint32_t SerialPort::get_remain_bytes() const
//{
//    return buffer_->get_remain_bytes();
//}

void SerialPort::notify_me_when_disconnected(std::function<void (int)> func)
{
    disconnect_connections_.connect(func);
}

void SerialPort::dont_buffer_notify_me_data_received(std::function<void (const char *, size_t)> func)
{
    data_received_connections_.connect(func);
    is_buffered_data_ = false;
}

int SerialPort::send(const std::string &buf)
{
    return send(buf.c_str(), buf.size());
}

int SerialPort::send(const char *buf, const int &size)
{
    boost::system::error_code ec;

    if (!port_)
        return -1;
    if (size == 0)
        return 0;

    return port_->write_some(boost::asio::buffer(buf, size), ec);
}

void SerialPort::async_send(const std::string &buf, std::function<void (int)> func)
{
    async_send(buf.data(), buf.size(), func);
}

void SerialPort::async_send(const char *buf, const int &size, std::function<void (int)> func)
{
    if (!port_)
        return;
    if (size == 0)
        return;

    port_->async_write_some(boost::asio::buffer(buf, size), boost::bind(func, boost::asio::placeholders::bytes_transferred));
}

void SerialPort::async_read_some()
{
    if (port_.get() == NULL || !port_->is_open())
        return;

    port_->async_read_some(
                boost::asio::buffer(data_, SERIAL_PORT_READ_BUF_SIZE),
                boost::bind(
                    &SerialPort::handle_read_data,
                    this, boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void SerialPort::handle_read_data(const boost::system::error_code& ec, size_t bytes_transferred)
{
    boost::mutex::scoped_lock look(mutex_);

    if (port_.get() == NULL || !port_->is_open()) return;
    if (ec) {
        async_read_some();
        return;
    }
    if (!is_buffered_data_) {
        data_received_connections_((const char*)data_.data(), bytes_transferred);
    } else {
//        buffer_->write(data_.data(), bytes_transferred);
    }
    async_read_some();
}

} // namespace peripheral
} // namespace hp
