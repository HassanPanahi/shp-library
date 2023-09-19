#ifndef TCPMESSAGEEXTRACTOR_H
#define TCPMESSAGEEXTRACTOR_H

#include "buffer/abstract_buffer.h"
#include "abstract_message.h"
#include "functional"

#include <thread>

namespace shp {
namespace network {

using Section_Handler_FuncPtr = std::function<PacketErrors ()>;

class MessageExtractor : public AbstractMessageExtractor
{
public:
    MessageExtractor(const std::vector<std::shared_ptr<Section>>& packet_structure, const uint32_t buffer_size_bytes = 4096, const uint32_t packet_buffer_count = 128);
    FindPacket get_next_message();
    std::shared_ptr<AbstractPacketStructure> get_packet_structure() const;
    PacketDefineErrors get_packet_error() const;
    void write_bytes(const uint8_t *data, const size_t size);

private:
    void add_wrong_header(const std::vector<uint8_t> &header);

    void start_extraction();
    void registre_commands();
    void find_header();
    uint32_t calc_len(const std::vector<uint8_t> data, const uint32_t size, bool is_msb);
    std::string get_next_bytes(uint32_t size);
    template<class Containter>
    void fill_packet(std::string& source, const Containter &data);

    PacketErrors handle_cmd_section();
    PacketErrors handle_crc_section();
    PacketErrors handle_data_section();
    PacketErrors handle_other_section();
    PacketErrors handle_header_section();
    PacketErrors handle_length_section();
    PacketErrors handle_footer_section();

    uint32_t packet_lenght_;
    std::vector<uint8_t> packet_header_;
    std::vector<uint8_t> packet_data_;
    std::vector<uint8_t> packet_cmd_;
    std::vector<uint8_t> packet_length_;
    std::vector<uint8_t> packet_footer_;
    std::vector<uint8_t> packet_other_;
    std::vector<uint8_t> packet_crc_;

//    std::vector<Section_Shared> packet_sections_;
    std::shared_ptr<AbstractSerializableMessage> current_msg_;
    std::shared_ptr<AbstractBuffer<uint8_t>> buffer_;
    std::shared_ptr<AbstractBuffer<FindPacket>> buffer_packet_;
    std::shared_ptr<AbstractPacketStructure> packet_structure_;
    std::map<PacketSections, Section_Handler_FuncPtr> sections_map_;
    std::thread analyze_thread_;

    FindPacket current_packet_;


};

} // namespace peripheral
} // namespace hp

#endif // TCPMESSAGEEXTRACTOR_H
