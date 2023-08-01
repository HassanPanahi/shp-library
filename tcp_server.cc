#include "tcp_server.h"
#include <iostream>

namespace hp {
namespace peripheral {

TCPServer::TCPServer(int port)
    : port_(port)
{
    io_context_ = boost::make_shared<boost::asio::io_context>();
    client_number_ = 0;
    is_running_ = false;
    accept_connection_ = true;
}

void TCPServer::start()
{
    if (!is_running_) {
        try {
            acceptor_ = boost::make_shared<boost::asio::ip::tcp::acceptor>(*io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));
            handle_connection();
            worker_thread_ = boost::make_shared<boost::thread>(&TCPServer::worker_thread,this);
            is_running_ = true;
        }  catch (boost::wrapexcept<boost::system::system_error>& exp) {
            std::cout << "error: " << exp.what() << std::endl;
        }
    }
}

void TCPServer::stop()
{
    is_running_ = false;
    acceptor_.reset();
}

void TCPServer::notify_me_for_new_connection(std::function<void (TCPClientShared)> func)
{
    client_object_connections_.connect(func);
}

void TCPServer::send_to_all_clients(const char *data, size_t size)
{
    clients_mutex_.lock();
    for(auto client: all_clients_map_) {
        if (client.second->is_connected()) {
            client.second->send(data, size);
        } else {
            std::cout << "client's closed" << std::endl;
        }
    }
    clients_mutex_.unlock();
}

void TCPServer::send_to_client(const char *data, size_t size, uint32_t id)
{
    auto it = all_clients_map_.find(id);
    if (it != all_clients_map_.end()) {
        if (it->second->is_connected())
            it->second->send(data, size);
    }
}

TCPServer::~TCPServer()
{
    std::cout << "~TCPServer" << std::endl;
    io_context_->stop();
    if (is_running_) {
        acceptor_->cancel();
        acceptor_.reset();
    }
    worker_thread_->join();
}

void TCPServer::disconnect(int id)
{
    clients_mutex_.lock();
    auto it = all_clients_map_.find(id);
    if (it != all_clients_map_.end())
        all_clients_map_.erase(it);
    clients_mutex_.unlock();
}

void TCPServer::handle_connection()
{
    auto socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*io_context_);
    acceptor_->async_accept(*socket_, boost::bind(&TCPServer::handle_accept, this, socket_, boost::asio::placeholders::error()));
}

void TCPServer::accept_connection(bool state)
{
    accept_connection_ = state;
}

void TCPServer::disconnect_client(const uint32_t id)
{
    clients_mutex_.lock();
    auto it = all_clients_map_.find(id);
    if (it != all_clients_map_.end())
        if (it->second->is_connected())
            it->second->disconnect();
    clients_mutex_.unlock();
}

//void TCPServer::dont_buffer_notify_me_data_received(std::function<void (char * data, size_t size, uint32_t id)> func)
//{
//    received_data_func_ = func;
//}

TCPClientShared TCPServer::get_client(uint32_t id)
{
    TCPClientShared client = nullptr;
    clients_mutex_.lock();
    auto it = all_clients_map_.find(id);
    if (it != all_clients_map_.end())
        client = it->second;
    clients_mutex_.unlock();
    return client;
}

void TCPServer::handle_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &error)
{
    try {
        if(!error && accept_connection_) {
            auto tcp_client = std::make_shared<TCPClient>(socket);
            clients_mutex_.lock();
            all_clients_map_[tcp_client->get_client_id()] = tcp_client;
            clients_mutex_.unlock();
            tcp_client->connect();
            tcp_client->notify_me_when_disconnected(std::bind(&TCPServer::disconnect, this, std::placeholders::_1));
            client_object_connections_(tcp_client);
            client_number_++;
            handle_connection();
        }
    }  catch (std::exception &e) {
        std::cout << "fuck: " << e.what() << std::endl;
    }  catch (boost::exception &e) {
        std::cout << "fuck2: " << std::endl;
    } catch(...) {
        std::cout << "fuck3" << std::endl;
    }
}

void TCPServer::worker_thread()
{
    boost::system::error_code ec;
    io_context_->run(ec);
}

} // namespace peripheral
} // namespace hp

