#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/signals2.hpp>

#include "abstract_buffer.h"
#include "buffer_template.h"
#include "abstract_message.h"

typedef boost::shared_ptr<boost::asio::serial_port> SerialPortShared;

#define SERIAL_PORT_READ_BUF_SIZE 256





namespace hp {
namespace peripheral {

enum SerialBaudRate {
    _110    = 110,
    _300    = 300,
    _600    = 600,
    _1200   = 1200,
    _2400   = 2400,
    _4800   = 4800,
    _9600   = 9600,
    _14400  = 14400,
    _19200  = 19200,
    _38400  = 38400,
    _57600  = 57600,
    _115200 = 115200,
    _128000 = 128000,
    _256000 = 256000
};


class SerialPort
{

public:
	SerialPort(void);
    ~SerialPort(void);

    bool start(const std::string &port, SerialBaudRate baud_rate= SerialBaudRate::_115200);
    void stop();

    void set_buffer(std::shared_ptr<AbstractBuffer> buffer);
    void set_buffer_size(uint32_t size);

    void extract_messages(std::shared_ptr<AbstractPacketStructure> extractor);
    std::shared_ptr<AbstractSerializableMessage> get_next_packet();
    BufferError get_next_bytes(uint8_t *data, const uint32_t len, const uint32_t timeout_ms = 0);
    uint8_t get_next_byte();
    std::string get_all_bytes();
    uint32_t get_remain_bytes() const;

    void notify_me_when_disconnected(std::function<void (int) >func);
    void dont_buffer_notify_me_data_received(std::function<void (const char * data, size_t size)> func);


    int send(const std::string &buf);
    int send(const char *buf, const int &size);
    void async_send(const std::string &buf, std::function<void (int)> func);
    void async_send(const char *buf, const int &size, std::function<void (int)> func);

	static std::vector<std::string> get_port_names();
    static void print_ports();
    static bool is_port_accessible(std::string &port);

    void print_send_receive_rate();
    void print_send_rate();
    void print_receive_rate();
private:
    void extract_message();
    void async_read_some();
    void handle_read_data(const boost::system::error_code& ec, size_t bytes_transferred);

        bool is_running_;
    boost::asio::io_service io_service_;
    SerialPortShared port_;
    boost::mutex mutex_;
    uint32_t buffer_size_;
    bool is_set_buffer_;
    bool is_set_extractor_;
    Buffer<std::shared_ptr<AbstractSerializableMessage>> messages_buffer_;
    boost::signals2::signal <void (const char* data, size_t size)> data_received_connections_;
    boost::signals2::signal <void (size_t)> disconnect_connections_;
    bool is_buffered_data_;
    std::array<uint8_t, SERIAL_PORT_READ_BUF_SIZE> data_;
    boost::thread_group thread_group_;

    std::shared_ptr<AbstractBuffer> buffer_;
    std::shared_ptr<AbstractPacketStructure> serial_message_extractor_;
};

} // namespace peripheral
} // namespace hp
