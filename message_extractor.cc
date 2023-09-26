#include "message_extractor.h"
#include "buffer/bounded_buffer.h"
#include <iostream>

namespace shp {
namespace network {

MessageExtractor::MessageExtractor(const std::vector<std::shared_ptr<Section>> &packet_structure, const uint32_t buffer_size_bytes, const uint32_t packet_buffer_count)
{
    buffer_ = std::make_shared<BoundedBuffer<uint8_t>>(buffer_size_bytes);
    buffer_packet_ = std::make_shared<BoundedBuffer<FindPacket>>(packet_buffer_count);
    packet_structure_ = std::make_shared<ClientPacket>(packet_structure);
    registre_commands();
    is_running_ = true;
    analyze_thread_ = std::thread(&MessageExtractor::start_extraction, this);
    //    packet_structure_->init_packet();
    //    packet_structure_->is_packet_sections_correct(packet_sections_);
}

void MessageExtractor::add_wrong_header(const std::vector<uint8_t>& header)
{
    FindPacket wrong_header;
    wrong_header.packet.push_back({PacketSections::Header, header});
    wrong_header.error = PacketErrors::Wrong_Footer;
    buffer_packet_->write(wrong_header);
}

PacketErrors MessageExtractor::handle_header_section()
{
    uint32_t header_index = 0;
    FindPacket current_packet;

    std::vector<uint8_t> failed_header;
    packet_header_ = packet_structure_->get_header()->get_header();
    const auto header_size = packet_header_.size();
    while (1) {
        if (header_index == header_size)
            break;
        uint8_t header = buffer_->read();
        if (packet_header_[header_index] == header) {
            header_index++;
        } else {
            failed_header.push_back(header);
            header_index = 0;
        }
        if ((header_index != 0 && failed_header.size() > 0) || buffer_->get_remain() == 0 || failed_header.size() == header_size * 5) {
            add_wrong_header(failed_header);
            failed_header.clear();
        }
    }
    auto final_header = std::make_shared<HeaderSection>(packet_header_);
    current_packet_.packet.push_back({PacketSections::Header, packet_header_});
    return PacketErrors::NO_ERROR;
}

PacketErrors MessageExtractor::handle_cmd_section()
{
    PacketErrors result = PacketErrors::NO_ERROR;
    packet_cmd_ = buffer_->read(packet_structure_->get_cmd()->get_size());
    current_msg_ = packet_structure_->get_cmd()->get_factory()->build_message(packet_cmd_);
    if (current_msg_ == nullptr)
        result = PacketErrors::Wrong_Header;
    current_packet_.packet.push_back({PacketSections::CMD, packet_cmd_});
    return result;
}

PacketErrors MessageExtractor::handle_crc_section()
{
    PacketErrors result = PacketErrors::NO_ERROR;
    auto crc_section = packet_structure_->get_crc();
    packet_crc_ = buffer_->read(crc_section->get_size());
    current_packet_.packet.push_back({PacketSections::CRC, packet_crc_});

    std::map<PacketSections, std::vector<uint8_t>> crc_data;
    if (crc_section->get_include() == PacketSections::CMD)
        crc_data[PacketSections::CMD] = packet_cmd_;

    if (crc_section->get_include() == PacketSections::Data)
        crc_data[PacketSections::Data] = packet_data_;

    if (crc_section->get_include() == PacketSections::Header)
        crc_data[PacketSections::Header] = packet_header_;

    if (crc_section->get_include() == PacketSections::Length)
        crc_data[PacketSections::Length] = packet_length_;

    bool is_ok = crc_section->get_crc_checker()->is_valid(crc_data, packet_crc_);
    if (!is_ok)
        result = PacketErrors::Wrong_CRC;
    return result;
}

PacketErrors MessageExtractor::handle_length_section()
{
    //TODO(HP): check len va buffer len
    auto length_section = packet_structure_->get_length();
    packet_length_ = buffer_->read(length_section->get_size());
    packet_lenght_ = calc_len(packet_length_, length_section->get_size(), packet_structure_->get_length()->get_is_first_byte_msb());
    current_packet_.packet.push_back({PacketSections::Length, packet_length_});
    return PacketErrors::NO_ERROR;
}

PacketErrors MessageExtractor::handle_data_section()
{
    uint32_t data_size = 0;
    if (packet_structure_->is_length_exist()) {
        data_size = packet_lenght_;
    } else {
        if (current_msg_ != nullptr)
            data_size = current_msg_->get_serialize_size();
        else
            data_size = packet_structure_->get_data()->get_size();
    }
    packet_data_ = buffer_->read(data_size);
    current_packet_.packet.push_back({PacketSections::Data, packet_data_});

    return PacketErrors::NO_ERROR;
}

PacketErrors MessageExtractor::handle_other_section()
{
    auto ret = PacketErrors::NO_ERROR;
    return ret;
}

PacketErrors MessageExtractor::handle_footer_section()
{
    auto ret = PacketErrors::NO_ERROR;
    auto footer_section = packet_structure_->get_footer();
    packet_footer_ = buffer_->read(footer_section->get_footer().size());
    if (footer_section->get_footer() != packet_footer_)
        ret = PacketErrors::Wrong_Footer;
    current_packet_.packet.push_back({PacketSections::Footer, packet_footer_});

    return ret;
}

FindPacket MessageExtractor::get_next_message()
{
    return buffer_packet_->read();
}

std::shared_ptr<AbstractPacketStructure> MessageExtractor::get_packet_structure() const
{
    return packet_structure_;
}

PacketDefineErrors MessageExtractor::get_packet_error() const
{
    auto ret = PacketDefineErrors::PACKET_OK;
    return ret;
}

void MessageExtractor::write_bytes(const uint8_t *data, const size_t size)
{
    buffer_->write(data, size);
}

MessageExtractor::~MessageExtractor()
{
    is_running_ = false;
    if (analyze_thread_.joinable())
        analyze_thread_.join();
}

void MessageExtractor::start_extraction()
{
    while (is_running_) {
        current_packet_.error = PacketErrors::NO_ERROR;
        current_packet_.packet.clear();
        for (const auto &section : packet_structure_->get_packet_structure()) {
            auto type = section->get_type();
            auto command_section_itr = sections_map_.find(type);
            if (command_section_itr == sections_map_.end()) {
                std::cout << "command doesn't found" << std::endl;
            } else {
                auto section_func_ptr = command_section_itr->second;
                auto is_find = section_func_ptr();
                if (is_find != PacketErrors::NO_ERROR) {
                    current_packet_.error = is_find;
                    buffer_packet_->write(current_packet_);
                }
            }
        }
        buffer_packet_->write(current_packet_);
        int a = 0;
    }
}

void MessageExtractor::find_header()
{

}


uint32_t MessageExtractor::calc_len(const std::vector<uint8_t> data, const uint32_t size, bool is_msb)
{
    uint32_t len = 0;
    switch (size) {
    case 1 : {
        len = data[0];
        break;
    }
    case 2 : {
        if (is_msb)
            len = static_cast<uint32_t>((data[0]) << 8 | (data[1]));
        else
            len = static_cast<uint32_t>((data[1]) << 8 | (data[0]));
        break;
    }
    case 3 : {

    }
    case 4: {
        if (is_msb)
            len =  static_cast<uint32_t>((data[0]) << 24 | (data[1]) << 16 | (data[2]) << 8 | (data[3]));
        else
            len =  static_cast<uint32_t>((data[3]) << 24 | (data[2]) << 16 | (data[1]) << 8 | (data[0]));

    }
    case 5 : {

    }
    case 6 : {

    }
    case 7 : {

    }
    case 8 : {

    }
    }
    return len;
}


std::string MessageExtractor::get_next_bytes(uint32_t size)
{
    std::string data;
    data.resize(size);
    for (uint32_t i = 0; i < size; i++)
        data[i] = static_cast<char>(buffer_->read());
    return data;
}

void MessageExtractor::registre_commands()
{
    sections_map_[PacketSections::CMD] = std::bind(&MessageExtractor::handle_cmd_section, this);;
    sections_map_[PacketSections::CRC] = std::bind(&MessageExtractor::handle_crc_section, this);;
    sections_map_[PacketSections::Data] = std::bind(&MessageExtractor::handle_data_section, this);;
    sections_map_[PacketSections::Other] = std::bind(&MessageExtractor::handle_other_section, this);;
    sections_map_[PacketSections::Header] = std::bind(&MessageExtractor::handle_header_section, this);
    sections_map_[PacketSections::Length] = std::bind(&MessageExtractor::handle_length_section, this);;
    sections_map_[PacketSections::Footer] = std::bind(&MessageExtractor::handle_footer_section, this);;
}


template<class Containter>
void MessageExtractor::fill_packet(std::string& source, const Containter& data)
{
    source.resize(source.size() + data.size());
    std::copy(data.begin(), data.end(), source.end() - data.size());
}

} // namespace peripheral
} // namespace hp
