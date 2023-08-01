#include "udp_server.h"

namespace hp {
namespace peripheral {

UDPServer::UDPServer(short port)
    : port_(port)
{

}

bool UDPServer::start()
{

}

bool UDPServer::is_running() const
{
    return is_running_;
}

void UDPServer::dont_buffer_notify_me_data_received(std::function<void (const char *, size_t, uint32_t)> func)
{

}

int UDPServer::send(const std::string &data)
{

}

void UDPServer::set_buffer(std::shared_ptr<AbstractBuffer> buffer)
{

}

void UDPServer::extract_messages(std::shared_ptr<AbstractMessageExtractor> extractor)
{

}

void UDPServer::set_buffer_size(uint64_t size_bytes)
{

}

void UDPServer::handle_read(const boost::system::error_code &erro, size_t bytes_transferred)
{

}

} // namespace peripheral
} // namespace hp
