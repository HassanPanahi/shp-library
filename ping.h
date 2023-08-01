#ifndef PING_H
#define PING_H


#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <istream>
#include <iostream>
#include <ostream>
#include <boost/asio/deadline_timer.hpp>

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

using boost::asio::ip::icmp;
//using boost::asio::steady_timer;
//namespace chrono = boost::asio::chrono;
using boost::asio::deadline_timer;
namespace posix_time = boost::posix_time;

class Ping
{
public:
    //TODO(HP): Change design. remove ip from constructor
    Ping(std::string ip);
    void ping_until_disconnect(int counter);
private:
    void start_send();
    void handle_timeout();
    void start_receive();
    void handle_receive(std::size_t length);
    int send_ping(int counter);
    static unsigned short get_identifier();
    boost::asio::io_service io_service_;

    std::string ip_;

    icmp::resolver resolver_;
    icmp::endpoint destination_;
    icmp::socket socket_;
    deadline_timer timer_;
    unsigned short sequence_number_;
    posix_time::ptime time_sent_;
    boost::asio::streambuf reply_buffer_;
    std::size_t num_replies_;
//    boost::asio::chrono::steady_clock::time_point time_sent_;
    int time_out_;
    int time_out_counter_;
};

#endif // PING_H
