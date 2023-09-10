#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include <boost/mpl/size.hpp>
#include "abstract_buffer.h"
#include "abstract_message.h"

#define MAX_LENGTH (1 * 1024 *  1024)

using BoostAsioTCPSocketShared = std::shared_ptr<boost::asio::ip::tcp::socket>;
using ReadFunctionPtr = boost::function<void (const boost::system::error_code error, const size_t bytes_transferred)>;

namespace shp {
namespace network {

class AbstractTCPClient
{
public:
    virtual void start() = 0;
    virtual unsigned short get_port() const = 0;
    virtual std::string get_ip() const = 0;
    virtual void disconnect() = 0;
    virtual size_t send(const char* data, const size_t size) = 0;
    virtual void async_send(const char* data, const size_t size, std::function<void (size_t)> func) = 0;
    virtual boost::signals2::connection notify_me_when_disconnected(std::function<void ()>& func) = 0;
    virtual boost::signals2::connection notify_me_when_data_received(std::function<void (const char * data, size_t size)> func) = 0;
    virtual boost::signals2::connection notify_me_when_new_packet_found(std::function<void (const char * data, size_t size)> func) = 0;
    virtual void set_extractor(std::shared_ptr<AbstractMessageExtractor> extractor) = 0;
    virtual void get_next_packet() = 0;
    virtual void set_extractor_packager() = 0;
    virtual bool is_connected() const = 0;
    virtual ~AbstractTCPClient() {}
};

class TCPClient : public AbstractTCPClient{
public:
    TCPClient(const BoostAsioTCPSocketShared& socket);
    TCPClient(const std::string &ip, const unsigned short port);
    void start();
    void disconnect();
    size_t send(const char* data, const size_t size);
    void async_send(const char* data, const size_t size, std::function<void (size_t)> func);

    std::string get_ip() const;
    unsigned short get_port() const;
    bool is_connected() const;


    boost::signals2::connection notify_me_when_disconnected(std::function<void ()>& func);
    boost::signals2::connection notify_me_when_data_received(std::function<void (const char * data, size_t size)> func);

    void do_buffer_data(const bool state);

    ~TCPClient();
private:
    bool connect();
    void initialize();
    void io_context_thread();
    void handle_read_data(const boost::system::error_code error, const size_t bytes_transferred);

    std::string ip_;
    unsigned short port_;
    uint64_t buffer_size_;
    bool is_connected_;
    bool is_running_;

    bool do_buffer_data_;

    BoostAsioTCPSocketShared socket_;
    boost::shared_ptr<boost::asio::io_context::work> work_;
    ReadFunctionPtr read_handler_;
    //TODO(HP): change this variabe to char * because code will be crash in big size
    std::array<uint8_t, MAX_LENGTH> data_;
    boost::signals2::signal <void (char* const data, const size_t size)> data_received_connections_;
    boost::signals2::signal <void ()> disconnect_connections_;

    std::shared_ptr<AbstractBuffer<uint8_t>> buffer_;
    std::mutex tcp_msg_extractor_lock_;
    boost::thread_group threads_group_;
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::endpoint endpoint_;

    // AbstractTCPClient interface
public:

    // AbstractTCPClient interface
public:
    boost::signals2::connection notify_me_when_new_packet_found(std::function<void (const char *, size_t)> func);
    void set_extractor(std::shared_ptr<AbstractMessageExtractor> extractor);
    void get_next_packet();
    void set_extractor_packager();
};

}
}

typedef std::shared_ptr<shp::network::TCPClient> TCPClientShared;

#endif // TCPCLIENT_H
