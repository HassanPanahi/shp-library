#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <boost/asio/ip/udp.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/signals2.hpp>

#include "hp/common/buffer/abstract_buffer.h"

#include "abstract_message.h"

#define MAX_LENGTH (40 * 1024 * 1024)

namespace hp {
namespace peripheral {

class UDPServer
{
public:
    UDPServer(short port);
    bool start();
    void stop();
    bool is_running() const;
    void dont_buffer_notify_me_data_received(std::function<void (const char * data, size_t size, uint32_t id)> func);

    int send(const std::string& data);
    int send(const char* data, const size_t size);

    void async_send(const std::string& data);
    void async_send(const char* data, const size_t size);

    void set_buffer(std::shared_ptr<AbstractBuffer> buffer);
    void extract_messages(std::shared_ptr<AbstractMessageExtractor> extractor);
    void set_buffer_size(uint64_t size_bytes);
    std::shared_ptr<AbstractSerializableMessage> get_next_packet();
    BufferError get_next_bytes(uint8_t *data, const uint32_t len, const uint32_t timeout_ms = 0);
    uint8_t get_next_byte();
    std::string get_all_bytes();
    uint32_t get_remain_bytes() const;
    void check_line_state();

private:
    void handle_read(const boost::system::error_code &erro, size_t bytes_transferred);
    boost::signals2::signal <void (const char* data, size_t size, uint32_t id)> data_received_connections_;

    short port_;
    boost::shared_ptr<boost::asio::io_context> io_context_;
    boost::shared_ptr<boost::asio::ip::udp::socket> udp_socket_;
    bool is_running_;
    boost::thread_group thread_group_;
    std::array<unsigned char, MAX_LENGTH> receive_buffer_;

    std::shared_ptr<AbstractBuffer> buffer_;
    std::shared_ptr<AbstractMessageExtractor> udp_message_extractor_;
};

} // namespace peripheral
} // namespace hp

#endif // UDPSERVER_H
