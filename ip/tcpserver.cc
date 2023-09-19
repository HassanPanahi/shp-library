#include "tcpserver.h"
#include <iostream>
#include <boost/chrono/system_clocks.hpp>


TCPServer::TCPServer(uint16_t port) : port_(port)
{
    io_service_ = std::make_shared<boost::asio::io_service>();
    work_ = std::make_shared<boost::asio::io_service::work>(*io_service_);
    accept_connections_ = true;
}

void TCPServer::start()
{
    is_running_ = true;
    thread_group_.create_thread(std::bind(&TCPServer::worker_thread, this, io_service_));
    acceptor_ = std::make_shared<boost::asio::ip::tcp::acceptor>(*io_service_,  boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));
    std::shared_ptr<boost::asio::ip::tcp::socket> temp = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_);
    acceptor_->async_accept(*temp, boost::bind(&TCPServer::handle_accept, this, temp, boost::asio::placeholders::error()));
    std::cout << "TCP server is ran in port: " << port_ << std::endl;
}

void TCPServer::send_data(const char *buffer, size_t size, int connection_number)
{
    std::lock_guard<std::mutex> lock(clients_map_lock_);
    try {
        auto client = clients_map_.find(connection_number);
        if (client != clients_map_.end())
            if (client->second->is_open())
                client->second->send(boost::asio::buffer(buffer, size));
    } catch(std::exception& ex) {
        std::cout << " Can't send to : " << connection_number << " bcus: " << ex.what() << std::endl;
    }
}

void TCPServer::send_data_to_this_server_sockets(const char *buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(clients_map_lock_);
    for (const auto &client : clients_map_) {
        try {
            client.second->send(boost::asio::buffer(buffer, size));
        } catch(std::exception& ex) {
            std::cout << " Cant send to all : " << client.first << " bcus: " << ex.what() << std::endl;
        }
    }
}

void TCPServer::close_connection(int connection_number)
{
    //TODO(HP) (add recursive mutex here)
    auto it =  clients_map_.find(connection_number);
    if (it != clients_map_.end())
        it->second->close();
}

void TCPServer::close_all_connections()
{
    std::lock_guard<std::mutex> lock(clients_map_lock_);
    for (int client_number: client_numbers_vec_) {
        close_connection(client_number);
        connections_socket_closed_(client_number);
    }
    clients_map_.clear();
    client_numbers_vec_.clear();
}

//void TCPServer::send_data_to_all_servers_sockets(const char *buffer, size_t size)
//{
//        for (auto client : all_clients_) {
//            try {
//                if (client == nullptr)
//                    continue;
//                if (client->is_open())
//                    client->send(boost::asio::buffer(buffer, size));
//            } catch(std::exception& ex) {
//                std::cout << ex.what() << std::endl;
//            }
//        }
//}

void TCPServer::connect_on_message_received(std::function<void (const char * msg, const size_t size, const int connection_number)> func)
{
    connections_message_received_.connect(func);
}

void TCPServer::connect_on_socket_closed(std::function<void (const int)> func)
{
    connections_socket_closed_.connect(func);
}

void TCPServer::set_accept_connection(bool state)
{
    accept_connections_ = state;
}

void TCPServer::stop_ping(const int connection_number)
{
    auto it = ping_classes_map_.find(connection_number);
    if (it != ping_classes_map_.end()) {
//            it->second->stop_ping();
            std::cout << "Stop ping for id: " << connection_number << std::endl;
    }
}

void TCPServer::send_ping_request(std::string ip, int connection_number)
{
    for (const auto &client_ip : ping_connections_map_) {
        if (client_ip.second == ip)
            return;
    }
    ping_connections_map_[connection_number] = ip;
//    auto ping = std::make_shared<Ping>(ip);
//    ping_classes_map_[connection_number] = ping;
//    std::cout << "Start to ping : " << ip << std::endl;
//    ping->ping_until_disconnect(5);

    std::lock_guard<std::mutex> lock(clients_map_lock_);
//    auto ping_obj = ping_classes_map_.find(connection_number);
//    ping_classes_map_.erase(ping_obj);

//    auto ping_ip = ping_connections_map_.find(connection_number);
//    if (ping_ip != ping_connections_map_.end())
//        ping_connections_map_.erase(ping_ip);

    auto it =  clients_map_.find(connection_number);
    if (it != clients_map_.end()) {
        it->second->close();
        clients_map_.erase(it);
        connections_socket_closed_(connection_number);
    }
    std::cout << "Disconnect " << ip << " with id: " << connection_number << std::endl;
}

void TCPServer::worker_thread(std::shared_ptr<boost::asio::io_service> io_service)
{
    try {
        while(1) {
            boost::system::error_code ec;
            io_service->run(ec);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch(std::exception& ex) {
        std::cout << "Worker not work: " << ex.what() << std::endl;
    }
}

void TCPServer::handle_read(const boost::system::error_code &error, size_t bytes_transferred, int connection_number)
{
    std::lock_guard<std::mutex> lock(clients_map_lock_);
    auto it =  clients_map_.find(connection_number);
    try {
        if (boost::asio::error::eof == error) {
            it->second->close();
            if (it != clients_map_.end()) {
                stop_ping(connection_number);
                clients_map_.erase(it);
                connections_socket_closed_(connection_number);
            }
            std::cout << "Disconnect from tcp client(End packet) id#" << connection_number << std::endl;
        } else {
            if (it != clients_map_.end()) {
                clients_map_[connection_number]->async_receive(boost::asio::buffer(receive_data_, TCP_MAX_PACKET_LEN),
                                                               boost::bind(&TCPServer::handle_read, this, boost::asio::placeholders::error ,boost::asio::placeholders::bytes_transferred, connection_number));
                connections_message_received_(receive_data_, bytes_transferred, connection_number);
            }
        }
    } catch(std::exception& ex) {
        std::cout <<" Handle read exeptrion: " << ex.what() << std::endl;
    }
}

void TCPServer::handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> client, const boost::system::error_code &error)
{
    if(!error && accept_connections_) {
        static int client_number = 0;
        client_numbers_vec_.push_back(client_number);
        std::cout << "Accept one connection with id #" << client_number << "  and ip: " << client->remote_endpoint().address().to_string() <<  std::endl;
        clients_map_[client_number] = client;
        clients_map_[client_number]->async_receive(boost::asio::buffer(receive_data_, TCP_MAX_PACKET_LEN),
                                                   boost::bind(&TCPServer::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, client_number));
        thread_group_.create_thread(std::bind(&TCPServer::send_ping_request, this, client->remote_endpoint().address().to_string(), client_number));
        client_number++;
    } else {
        if (!accept_connections_)
            std::cout << "Ignore client request because of accept_connection: " << client->remote_endpoint().address().to_string() <<  std::endl;
        else
            std::cout << "Ignore client request because of error: " << error.message() <<  " val: " << error.value() <<  std::endl;

        client->close();
    }
    std::shared_ptr<boost::asio::ip::tcp::socket> temp = std::make_shared<boost::asio::ip::tcp::socket>( *io_service_ );
    acceptor_->async_accept(*temp, boost::bind(&TCPServer::handle_accept, this, temp, boost::asio::placeholders::error()));
}

