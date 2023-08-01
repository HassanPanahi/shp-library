#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include <boost/mpl/size.hpp>
#include "hp/common/buffer/abstract_buffer.h"
#include "message_extractor.h"
#include "hp/common/buffer/buffer_template.h"

#define MAX_LENGTH (1 * 1024 *  1024)

typedef std::shared_ptr<boost::asio::ip::tcp::socket> TCPSocketShared;

namespace hp {
namespace peripheral {

enum class TCPError{
    CantSetNewBuffer,
    CantSetNewBufferSize,
    NoError
};

//class ClientConfig {
//public:
//    ClientConfig(std::string ip, short port) : ip_(ip), port_(port) {}
//    short get_port() { return port_; }
//    std::string get_ip() { return ip_; }

//private:
//    std::string ip_;
//    short port_;
//    uint64_t buffer_size_;
//    std::shared_ptr<AbstractBuffer> buffer_;
//    std::shared_ptr<AbstractPacketStructure> msg_extractor_;
//};

class TCPClient {
public:
    TCPClient(TCPSocketShared socket);
    TCPClient(std::string ip, short port);

    void async_connect(std::function<void (int id) > func);
    bool connect();
    void disconnect();
    size_t send(const char* data, const uint32_t size);
    void async_send(const char* data, const uint32_t size, std::function<void (size_t)> func);

    std::string get_ip() const;
    short get_port() const;
    int get_client_id() const;
    bool is_connected() const;

    boost::signals2::connection notify_me_when_disconnected(std::function<void (int) >func);
    boost::signals2::connection notify_me_data_received(std::function<void (const char * data, size_t size)> func);

    void set_buffer(std::shared_ptr<AbstractBuffer> buffer);
    void start_find_messages(std::shared_ptr<AbstractPacketStructure> extractor);
    void set_buffer_size(size_t size_bytes);
    std::shared_ptr<AbstractSerializableMessage> get_next_packet();
    BufferError read_next_bytes(char *data, const uint32_t len, const uint32_t timeout_ms = 0);
    char read_next_byte();
    std::string read_all_bytes();
    uint32_t read_remain_bytes();
    void do_buffer_data(const bool state);
    void print_send_receive_rate();
    void print_send_rate();
    void print_receive_rate();

    ~TCPClient();
private:
    void initialize();
    void handle_read_data(const boost::system::error_code error, const size_t bytes_transferred);
    void io_context_thread();
    void extract_message();

    std::string ip_;
    short port_;
    uint64_t buffer_size_;
    bool is_connected_;
    bool is_running_;
//    bool is_socket_create_by_server_;
    bool do_buffer_data_;
    int id_;
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    boost::shared_ptr<boost::asio::io_context::work> work_;

    //TODO(HP): change this variabe to char * because code will be crash in big size
    std::array<uint8_t, MAX_LENGTH> data_;
    boost::signals2::signal <void (const char* data, size_t size)> data_received_connections_;
    boost::signals2::signal <void (size_t)> disconnect_connections_;

    std::atomic<uint64_t> receive_size_;
    std::atomic<uint64_t> send_size_;

    Buffer<std::shared_ptr<AbstractSerializableMessage>> messages_buffer_;
    boost::thread_group thread_group_;
    boost::asio::ip::tcp::endpoint endpoint_;
    boost::asio::io_context io_context_;
    std::mutex tcp_msg_extractor_lock_;
    static std::atomic<int> ID_Counter_;
//    bool is_buffered_data_;
    bool buffer_is_mine_;
    std::shared_ptr<AbstractBuffer> buffer_;
    std::shared_ptr<AbstractPacketStructure> packet_structure_;
    std::shared_ptr<MessageExtractor> tcp_message_extractor_;
};

} // namespace peripheral
} // namespace hp

typedef std::shared_ptr<hp::peripheral::TCPClient> TCPClientShared;

#endif // TCPCLIENT_H
