#include "message_extractor.h"
#include "buffer/bounded_buffer.h"
#include <iostream>

namespace shp {
namespace network {

MessageExtractor::MessageExtractor(const std::shared_ptr<AbstractPacketStructure> &packet_structure, const uint32_t buffer_size_bytes, const uint32_t packet_buffer_count)
    : packet_structure_(packet_structure)
{
    buffer_ = std::make_shared<BoundedBuffer<uint8_t>>(buffer_size_bytes);
    packet_sections_ = packet_structure_->get_packet_structure();
    buffer_packet_ = std::make_shared<BoundedBuffer<FindPacket>>(packet_buffer_count);
    //    packet_structure_->init_packet();
    //    packet_structure_->is_packet_sections_correct(packet_sections_);
}

void MessageExtractor::add_wrong_header(const Section_Shared& header)
{
    FindPacket wrong_header;
    wrong_header.packet = header;
    wrong_header.error = PacketErrors::Wrong_Footer;
    buffer_packet_->write(wrong_header);
}

PacketErrors MessageExtractor::handle_header_section()
{
    uint32_t header_index = 0;
    auto failed_header = std::make_shared<HeaderSection>();
    auto find_header = std::make_shared<HeaderSection>();
    auto header_section = packet_structure_->get_header();
    const auto header_size = header_section->content.size();
    find_header->content.resize(header_size);
    while (1) {
        if (header_index == header_size)
            break;
        uint8_t header = buffer_->read();
        find_header->content.push_back(header);
        if (header_section->content[header_index] == header) {
            header_index++;
        } else {
            if (failed_header->content.size() >= header_size * 5) {
                add_wrong_header(failed_header);
                failed_header->content.clear();
            }
            failed_header->content.push_back(header);
            header_index = 0;
        }
    }

    if (failed_header->content.size() > 0)
        add_wrong_header(failed_header);

    find_packet_.push_back(find_header);
    return PacketErrors::NO_ERROR;
}

PacketErrors MessageExtractor::handle_cmd_section()
{
    PacketErrors result = PacketErrors::NO_ERROR;
    auto cmd = buffer_->read(packet_structure_->get_cmd()->size_bytes);
    current_msg_ = packet_structure_->get_cmd()->msg_factory->build_message(cmd);
    if (current_msg_ == nullptr)
        result = PacketErrors::Wrong_Header;
    return result;
}

PacketErrors MessageExtractor::handle_crc_section()
{
    //    crc = get_next_bytes(extractor_->get_crc()->size_bytes);

}

PacketErrors MessageExtractor::handle_length_section()
{
    //TODO(HP): check len va buffer len
    auto length_section = packet_structure_->get_length();
    auto length_data = buffer_->read(length_section->size_bytes);
    packet_lenght_ = calc_len(length_data, length_section->size_bytes, extractor_->get_length()->is_first_byte_msb);
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
            data_size = packet_structure_->get_data()->fix_size_bytes;
    }
    packet_data_ = buffer_->read(data_size);
    return PacketErrors::NO_ERROR;
}

PacketErrors MessageExtractor::handle_other_section()
{

}

PacketErrors MessageExtractor::handle_footer_section()
{
    //    footer = get_next_bytes(extractor_->get_footer()->content.size());
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

}

void MessageExtractor::write_bytes(const uint8_t *data, const size_t size)
{
    buffer_->write(data, size);
}

void MessageExtractor::start_extraction()
{
    while (1) {
        FindPacket msg;
        for (const auto &section : packet_sections_) {
            auto type = section->get_type();
            auto command_section_itr = sections_map_.find(type);
            if (command_section_itr == sections_map_.end()) {
                std::cout << "command doesn't found" << std::endl;
            } else {
                auto section_func_ptr = command_section_itr->second;
                bool is_find = section_func_ptr();
                if (!is_find)
                    buffer_packet_->write(msg);
            }
        }
        buffer_packet_->write(msg);
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
    //    sections_map_[PacketSections::Header] = std::bind();
    //    sections_map_[PacketSections::Length ] = std::bind();
    //        sections_map_[PacketSections::CMD ] = std::bind();

    //        sections_map_[PacketSections::Data] = std::bind();
    //        sections_map_[PacketSections::CRC] = std::bind();
    //        sections_map_[PacketSections::Footer] = std::bind();
    //        sections_map_[PacketSections::Other] = std::bind();

}


template<class Containter>
void MessageExtractor::fill_packet(std::string& source, const Containter& data)
{
    source.resize(source.size() + data.size());
    std::copy(data.begin(), data.end(), source.end() - data.size());
}

} // namespace peripheral
} // namespace hp
