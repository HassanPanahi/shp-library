#ifndef TCPASYNC_H
#define TCPASYNC_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <thread>
#include "tcp_client.h"

namespace hp {
namespace peripheral {

class TCPServer
{
public:
    TCPServer(int port);
    void start();
    void stop();
    bool is_running() const;
    void notify_me_for_new_connection(std::function<void (TCPClientShared)> func);
    void notify_me_for_set_client_config(std::function<void (TCPClientShared)> func);

    void send_to_all_clients(const char *data, size_t size);
    void send_to_client(const char* data, size_t size, uint32_t id);
    void accept_connection(bool state);
    void disconnect_client(const uint32_t id);

    TCPClientShared get_client(uint32_t id);
    ~TCPServer();

private:
    void disconnect(int id);
    void remove_class();
    void handle_connection();
    void handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &error);
    void worker_thread();

    int port_;
    bool is_running_;
    bool accept_connection_;
    uint32_t client_number_;
    std::map<int, TCPClientShared> all_clients_map_;
    std::mutex clients_mutex_;
    boost::shared_ptr<boost::asio::io_context> io_context_;
    boost::shared_ptr<boost::thread> worker_thread_;
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    boost::signals2::signal<void (TCPClientShared)> client_object_connections_;

};

} // namespace peripheral
} // namespace hp

#endif // TCPASYNC_H
