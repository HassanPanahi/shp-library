#include "message_extractor.h"
#include "bounded_buffer.h"
#include <iostream>

namespace shp {
namespace network {

MessageExtractor::MessageExtractor(const std::shared_ptr<AbstractPacketStructure> &packet_structure, const uint32_t buffer_size_bytes)
    : packet_structure_(packet_structure)
{
    buffer_ = std::make_shared<BoundedBuffer<uint8_t>>(buffer_size_bytes);
    packet_sections_ = packet_structure_->get_packet_structure();

    //    packet_structure_->init_packet();
    //    packet_structure_->is_packet_sections_correct(packet_sections_);
}

void MessageExtractor::handle_header_section()
{
    uint32_t header_index = 0;
    auto header_section = packet_structure_->get_header();
    while(1) {
        if (header_index == header_section->content.size())
            break;
        uint8_t header = buffer_->read();
        if (header_section->content[header_index] == header)
            header_index++;
        else {
            header_index = 0;
        }
    }
}

void MessageExtractor::handle_cmd_section()
{
    //    cmd = get_next_bytes(extractor_->get_cmd()->size_bytes);
    //    msg = extractor_->get_cmd()->msg_factory->build_message(cmd.data());
    //    if (msg == nullptr) {
    //        packet_sections[PacketSections::Header] = extractor_->get_header()->content;
    //        packet_sections[PacketSections::CMD] = cmd;
    //        packet_structure_->packet_section_error(PacketErrors::Wrong_CMD, packet_sections);
    //        is_cmd_null = true;
    //        continue;
    //    }
}

void MessageExtractor::handle_crc_section()
{
    //    crc = get_next_bytes(extractor_->get_crc()->size_bytes);

}

void MessageExtractor::handle_length_section()
{
    //    len = get_next_bytes(extractor_->get_length()->size_bytes);
    //    uint32_t len_val = calc_len(len.data(), extractor_->get_length()->size_bytes, extractor_->get_length()->is_first_byte_msb);
    //    data_len = len_val;
    //    can_find_length = true;
}

void MessageExtractor::handle_data_section()
{
    //    if (extractor_->is_length_exist()) {
    //        data.resize(data_len);
    //        data_size = data_len;
    //    } else {
    //        if (msg != nullptr)
    //            data_size = msg->get_serialize_size();
    //        else
    //            data_size = extractor_->get_data()->fix_size_bytes;
    //        data.resize(data_size);
    //    }
    //    buffer_->read((uint8_t*)data.data(), data_size);
}

void MessageExtractor::handle_other_section()
{

}

void MessageExtractor::handle_footer_section()
{
    //    footer = get_next_bytes(extractor_->get_footer()->content.size());
}


std::shared_ptr<AbstractPacketStructure> MessageExtractor::get_next_message()
{
    std::shared_ptr<AbstractPacketStructure> msg = nullptr;
    bool is_get_next_message = false;
    while (!is_get_next_message) {

        for (const auto &section : packet_sections_) {
            //            if (is_cmd_null || !is_footer_ok)
            //                break;
            auto type = section->get_type();


            auto command_section_itr = sections_map_.find(type);
            if (command_section_itr == sections_map_.end() ) {
                std::cout << "command doesn't found" << std::endl;
            } else {
                auto section_func_ptr = command_section_itr->second;
                section_func_ptr();
            }
        }
        //                if (is_crc_exist_) {
        //                    if (crc_include_data_)
        //                        packet_sections[PacketSections::Data] = data;
        //                    if (crc_include_cmd_)
        //                        packet_sections[PacketSections::CMD] = cmd;
        //                    if (crc_include_footer_)
        //                        packet_sections[PacketSections::Footer] = footer;
        //                    if (crc_include_header_)
        //                        packet_sections[PacketSections::Header] = data;
        //                    if (crc_include_crc_)
        //                        packet_sections[PacketSections::CRC] = crc;
        //                    if (crc_include_length_)
        //                        packet_sections[PacketSections::Length] = len;
        //                    is_crc_ok = crc_->crc_checker->is_valid(packet_sections, crc);
        //                    if (!is_crc_ok) {
        //                        packet_structure_->packet_section_error(PacketErrors::Wrong_CRC, packet_sections);
        //                        continue;
        //                    }
        //                }
        //                if (is_crc_ok && is_footer_ok && msg != nullptr) {
        //                    msg->deserialize((char*)data.data(), data_size);
        //                    is_get_next_message = true;
        //                }
    }
    return msg;
}

PacketDefineErrors MessageExtractor::get_packet_error() const
{

}

void MessageExtractor::write_bytes(const uint8_t *data, const size_t size)
{
    buffer_->write(data, size);
}

void MessageExtractor::find_header()
{

}


uint32_t MessageExtractor::calc_len(const char * data, uint32_t size, bool is_msb)
{
    uint32_t len = 0;
    switch (size) {
    case 1 : {
        len = *reinterpret_cast<uint32_t*>(*data);
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
