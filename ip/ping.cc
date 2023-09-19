#include "ping.h"
#include <future>


Ping::Ping(std::string ip)
    : ip_(ip) , resolver_(io_service_), socket_(io_service_, icmp::v4()), timer_(io_service_)
{
    icmp::resolver::query query(icmp::v4(), ip_, "");
    destination_ = *resolver_.resolve(query);
    sequence_number_ = 0;
    num_replies_ = 0;
}

void Ping::ping_until_disconnect(int counter)
{
    time_out_counter_ = counter;
    time_out_ = 0;
    start_send();
    start_receive();
    io_service_.run();
}

void Ping::start_send()
{
    std::string body("\"Hello!\" from Asio ping.");

    // Create an ICMP header for an echo request.
    icmp_header echo_request;
    echo_request.type(icmp_header::echo_request);
    echo_request.code(0);
    echo_request.identifier(get_identifier());
    echo_request.sequence_number(++sequence_number_);
    compute_checksum(echo_request, body.begin(), body.end());

    // Encode the request packet.
    boost::asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << echo_request << body;

    // Send the request.
    time_sent_ = posix_time::microsec_clock::universal_time();
    socket_.send_to(request_buffer.data(), destination_);

    // Wait up to five seconds for a reply.
    num_replies_ = 0;
    timer_.expires_at(time_sent_ + posix_time::seconds(2));
    timer_.async_wait(boost::bind(&Ping::handle_timeout, this));
}

void Ping::handle_timeout()
{
    if (num_replies_ == 0) {
        std::cout << ip_ <<" Request timed out " << std::endl;
        time_out_++;
        if (time_out_ == time_out_counter_) {
            timer_.cancel();
            io_service_.stop();
            return;
        }
    } else {
        time_out_ = 0;
    }
    // Requests must be sent no less than one second apart.
    timer_.expires_at(time_sent_ + posix_time::seconds(1));
    timer_.async_wait(boost::bind(&Ping::start_send, this));
}

void Ping::start_receive()
{
    // Discard any data already in the buffer.
    reply_buffer_.consume(reply_buffer_.size());

    // Wait for a reply. We prepare the buffer to receive up to 64KB.
    socket_.async_receive(reply_buffer_.prepare(65536),
                          boost::bind(&Ping::handle_receive, this, _2));
}

void Ping::handle_receive(std::size_t length)
{
    // The actual number of bytes received is committed to the buffer so that we
    // can extract it using a std::istream object.
    reply_buffer_.commit(length);

    // Decode the reply packet.
    std::istream is(&reply_buffer_);
    ipv4_header ipv4_hdr;
    icmp_header icmp_hdr;
    is >> ipv4_hdr >> icmp_hdr;

    // We can receive all ICMP packets received by the host, so we need to
    // filter out only the echo replies that match the our identifier and
    // expected sequence number.
    if (is && icmp_hdr.type() == icmp_header::echo_reply
            && icmp_hdr.identifier() == get_identifier()
            && icmp_hdr.sequence_number() == sequence_number_)
    {
        // If this is the first reply, interrupt the five second timeout.
        num_replies_++;
//        if (num_replies_++ == 0)
            timer_.cancel();
        // Print out some information about the reply packet.
//        auto now = std::chrono::steady_clock::now();
//        auto elapsed = now - time_sent_;
//        std::cout << length - ipv4_hdr.header_length()
//                  << " bytes from " << ipv4_hdr.source_address()
//                  << ": icmp_seq=" << icmp_hdr.sequence_number()
//                  << ", ttl=" << ipv4_hdr.time_to_live()
//                  << ", time="
//                  << chrono::duration_cast<chrono::milliseconds>(elapsed).count()
//                  << std::endl;
    }
    start_receive();
}

int Ping::send_ping(int counter)
{
    int reply_count = 0;
    for (int count = 0; count < counter; count++) {


    }
    return reply_count;
}

unsigned short Ping::get_identifier()
{
#if defined(BOOST_ASIO_WINDOWS)
    return static_cast<unsigned short>(::GetCurrentProcessId());
#else
    return static_cast<unsigned short>(::getpid());
#endif
}
