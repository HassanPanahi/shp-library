#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <thread>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/signals2/signal.hpp>

#include "ping.h"

#define TCP_MAX_PACKET_LEN (1000)

class TCPServer
{
public:
    TCPServer(uint16_t port);
    void start();
    void send_data(const char *buffer, size_t size, int connection_number);
    void send_data_to_this_server_sockets(const char *buffer, size_t size);
//    static void send_data_to_all_servers_sockets(const char *buffer, size_t size);
    void close_connection(int connection_number);
    void close_all_connections();
    void connect_on_message_received(std::function<void (const char * msg, const size_t size, const int connection_number)> func);
    void connect_on_socket_closed(std::function<void (const int connection_number)> func);
    void set_accept_connection(bool state);

private:
    void send_ping_request(std::string ip, int connection_number);
    std::shared_ptr<boost::asio::io_service> io_service_;
    std::shared_ptr<boost::asio::io_service::work> work_;
    boost::thread_group thread_group_;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    std::shared_ptr<boost::asio::ip::tcp::socket> client_;
    bool is_running_;
    char receive_data_[TCP_MAX_PACKET_LEN];
    void worker_thread(std::shared_ptr<boost::asio::io_service> io_service);
    void handle_read(const boost::system::error_code &error, size_t bytes_transferred, int connectionnumber);
    void handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> client,const boost::system::error_code &error);
    void stop_ping(const int connection_number);
    std::map<int, std::shared_ptr<boost::asio::ip::tcp::socket> > clients_map_;
    std::map<int, std::string> ping_connections_map_;
    std::map<int, std::shared_ptr<Ping>> ping_classes_map_;

    std::vector <int> client_numbers_vec_;
    std::mutex clients_map_lock_;
    boost::signals2::signal <void (const char* msg, const size_t size, const int connection_number)> connections_message_received_;
    boost::signals2::signal <void (const int connection_number)> connections_socket_closed_;

    uint16_t port_;
    bool accept_connections_;
};

#endif // TCPSERVER_H
