#ifndef TCPMESSAGEEXTRACTOR_H
#define TCPMESSAGEEXTRACTOR_H

#include "abstract_buffer.h"
#include "abstract_message.h"
#include "functional"

namespace shp {
namespace network {

using Section_Handler_FuncPtr = std::function<void ()>;

class MessageExtractor : public AbstractMessageExtractor
{
public:
    MessageExtractor(std::shared_ptr<AbstractPacketStructure> packet_structure);
    std::shared_ptr<AbstractSerializableMessage> find_message();
    std::shared_ptr<AbstractPacketStructure> get_packet_structure() const { return packet_structure_; }

private:
    void registre_commands();
    void find_header();
    int calc_len(const char *data, uint32_t size, bool is_msb);
    std::string get_next_bytes(uint32_t size);
    template<class Containter>
    void fill_packet(std::string& source, const Containter &data);

    void handle_header_section();
    void handle_cmd_section();
    void handle_crc_section();
    void handle_length_section();
    void handle_data_section();
    void handle_other_section();
    void handle_footer_section();

    std::shared_ptr<AbstractBuffer<uint8_t>> buffer_;
    std::vector<std::shared_ptr<Section>> packet_sections_;
    std::shared_ptr<AbstractPacketStructure> packet_structure_;
    std::map<PacketSections, Section_Handler_FuncPtr> sections_map_;
};

} // namespace peripheral
} // namespace hp

#endif // TCPMESSAGEEXTRACTOR_H
