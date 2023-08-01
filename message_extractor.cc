#include "message_extractor.h"
#include <iostream>

namespace hp {
namespace peripheral {

MessageExtractor::MessageExtractor(std::shared_ptr<AbstractPacketStructure> packet_structure,  std::shared_ptr<AbstractBuffer> buffer)
    : packet_structure_(packet_structure), buffer_(buffer)
{
    packet_structure_->init_packet();
    packet_sections_ = packet_structure_->get_packet_structure();
    packet_structure_->is_packet_sections_correct(packet_sections_);
}

std::shared_ptr<AbstractSerializableMessage> MessageExtractor::find_message()
{
    std::shared_ptr<AbstractSerializableMessage> msg = nullptr;
    bool is_find_message = false;
    while (!is_find_message) {
        std::map<PacketSections, std::string> crc_check_data;
        std::map<PacketSections, std::string> packet_sections;
        std::string footer;
        std::string packet;
        std::string data;
        std::string cmd;
        std::string len;
        std::string crc;
        bool is_footer_ok = true;
        bool is_cmd_null = false;
        bool is_crc_ok = true;
        bool can_find_length = false;
        uint32_t data_len = 0;
        uint32_t data_size = 0;

        for (auto section : packet_sections_) {
            if (is_cmd_null || !is_footer_ok)
                break;
            auto type = section->get_type();
            switch(type) {
            case PacketSections::Header : {
                find_header();
                break;
            }
            case PacketSections::CMD : {
                cmd = get_next_bytes(extractor_->get_cmd()->size_bytes);
                msg = extractor_->get_cmd()->msg_factory->build_message(cmd.data());
                if (msg == nullptr) {
                    packet_sections[PacketSections::Header] = extractor_->get_header()->content;
                    packet_sections[PacketSections::CMD] = cmd;
                    packet_structure_->packet_section_error(PacketErrors::Wrong_CMD, packet_sections);
                    is_cmd_null = true;
                    continue;
                }
                break;
            }
            case PacketSections::Length : {
                len = get_next_bytes(extractor_->get_length()->size_bytes);
                uint32_t len_val = calc_len(len.data(), extractor_->get_length()->size_bytes, extractor_->get_length()->is_first_byte_msb);
                data_len = len_val;
                can_find_length = true;
                break;
            }
            case PacketSections::Data : {
                if (extractor_->is_length_exist()) {
                    data.resize(data_len);
                    data_size = data_len;
                } else {
                    if (msg != nullptr)
                        data_size = msg->get_serialize_size();
                    else
                        data_size = extractor_->get_data()->fix_size_bytes;
                    data.resize(data_size);
                }
                buffer_->read((uint8_t*)data.data(), data_size);
                break;
            }
            case PacketSections::CRC : {
                crc = get_next_bytes(extractor_->get_crc()->size_bytes);
                break;
            }
            case PacketSections::Footer : {
                footer = get_next_bytes(extractor_->get_footer()->content.size());
                break;
            }
            case PacketSections::Other :{

                break;
            }
            }
        }
        //        if (is_crc_exist_) {
        //            if (crc_include_data_)
        //                packet_sections[PacketSections::Data] = data;
        //            if (crc_include_cmd_)
        //                packet_sections[PacketSections::CMD] = cmd;
        //            if (crc_include_footer_)
        //                packet_sections[PacketSections::Footer] = footer;
        //            if (crc_include_header_)
        //                packet_sections[PacketSections::Header] = data;
        //            if (crc_include_crc_)
        //                packet_sections[PacketSections::CRC] = crc;
        //            if (crc_include_length_)
        //                packet_sections[PacketSections::Length] = len;
        //            is_crc_ok = crc_->crc_checker->is_valid(packet_sections, crc);
        //            if (!is_crc_ok) {
        //                packet_structure_->packet_section_error(PacketErrors::Wrong_CRC, packet_sections);
        //                continue;
        //            }
        //        }
        //        if (is_crc_ok && is_footer_ok && msg != nullptr) {
        //            msg->deserialize((char*)data.data(), data_size);
        //            is_find_message = true;
        //        }
    }
    return msg;
}

std::shared_ptr<AbstractBuffer> MessageExtractor::get_buffer_() const
{
    return buffer_;
}

void MessageExtractor::find_header()
{
    //    uint32_t header_index = 0;
    //    while(1) {
    //        if (header_index == header_->content.size())
    //            break;
    //        char header = buffer_->read_next_byte();
    //        if (header_->content[header_index] ==  header)
    //            header_index++;
    //        else {
    //            header_index = 0;
    //        }
    //    }
}


int MessageExtractor::calc_len(const char * data, uint32_t size, bool is_msb)
{
    int len = 0;
    switch (size) {
    case 1 : {
        len = (int)(*data);
        break;
    }
    case 2 : {
        if (is_msb)
            len = int((unsigned char)(data[0]) << 8 | (unsigned char)(data[1]));
        else
            len = int((unsigned char)(data[1]) << 8 | (unsigned char)(data[0]));
        break;
    }
    case 3 : {

    }
    case 4: {
        if (is_msb)
            len =  int((unsigned char)(data[0]) << 24 | (unsigned char)(data[1]) << 16 |  (unsigned char)(data[2]) << 8 | (unsigned char)(data[3]));
        else
            len =  int((unsigned char)(data[3]) << 24 | (unsigned char)(data[2]) << 16 |  (unsigned char)(data[1]) << 8 | (unsigned char)(data[0]));

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
        data[i] = buffer_->read_next_byte();
    return data;
}

template<class Containter>
void MessageExtractor::fill_packet(std::string& source, const Containter& data)
{
    source.resize(source.size() + data.size());
    std::copy(data.begin(), data.end(), source.end() - data.size());
}

} // namespace peripheral
} // namespace hp
