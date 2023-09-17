#ifndef ABSTRACT_MESSAGE_H
#define ABSTRACT_MESSAGE_H

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "abstract_buffer.h"

namespace shp {
namespace network {

enum PacketSections {
    Header  = 1 << 0,
    Length  = 1 << 1,
    CMD     = 1 << 2,
    Footer  = 1 << 3,
    CRC     = 1 << 4,
    Data    = 1 << 5,
    Other   = 1 << 6
};

class AbstractCRC {
public:
    //!
    //! \brief is_valid check data in packet is valid or not.
    //! \param data include data section in packet
    //! \param data_size data section size in packet
    //! \param crc_data data of crc
    //! \param crc_size size of crc
    //! \return true mean the crc match with data.
    //!
    //!
    virtual bool is_valid(const std::map<PacketSections, std::string>& input_data, const std::string& data) = 0;
};

//!
//! \brief The AbstractSerializableMessage class is an interface which implement
//! the Message and used for any type of message that can serial and deseriale itself.
//!

class AbstractSerializableMessage {
public :
    //!
    //! \brief serialize method can fill the input buffer by serializing values .
    //!        the values in serializing and deserializing should be same.
    //! \param buffer for containg the serializing data
    //! \param size is used for available buffer size
    //!
    virtual void serialize(char* buffer, size_t size) = 0;
    //!
    //! \brief deserialize method is used for setting properties and members
    //!        from buffer as same method as serialize
    //! \param buffer is used for reading values
    //! \param size the values
    //!
    virtual void deserialize(const char * buffer, size_t size) = 0 ;
    //!
    //! \brief getserialSize method return the size of serializing and deserializing method
    //! \return the size_t value which show the serializ size this function will used when
    //!         packet not include packet_len
    //!
    virtual size_t get_serialize_size() const = 0;
    virtual int get_type() const = 0;
    virtual ~AbstractSerializableMessage(){} //! virtual interface destructor
};

class AbstractMessageFactory
{
public:
    //!
    //! \brief build_message is a factory for all messages that packet includes.
    //! \param cmd is a section that use in packet
    //! \return  must return a serializablemessage for save all messages
    //!
    virtual std::shared_ptr<AbstractSerializableMessage> build_message(const std::string cmd) = 0;
};



template<class T> inline T operator~ (T a) { return (T)~(int)a; }
template<class T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }
template<class T> inline T operator& (T a, T b) { return (T)((int)a & (int)b); }
template<class T> inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
template<class T> inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }


struct Section{
public:
    virtual PacketSections get_type() const = 0;
};

struct CRCSection : public Section {
    uint32_t size_bytes = 0;
    PacketSections include = PacketSections::Data;
    std::shared_ptr<AbstractCRC> crc_checker;
public:
    virtual PacketSections get_type()  const override { return PacketSections::CRC;}
};

struct LengthSection : public Section {
    uint32_t size_bytes = 0;
    PacketSections include = PacketSections::Data;
    bool is_first_byte_msb;
public:
    PacketSections get_type() const { return PacketSections::Length;}
};

struct HeaderSection : public Section {
    std::string content;
public:
    PacketSections get_type() const { return PacketSections::Header;}
};

struct FooterSection : public Section {
    std::string content;
public:
    PacketSections get_type() const { return PacketSections::Footer;}
};

struct CMDSection : public Section {
    uint32_t size_bytes = 0;
    std::shared_ptr<AbstractMessageFactory> msg_factory;
public:
    PacketSections get_type() const { return PacketSections::CMD;}
};

struct DataSection : public Section {
    uint32_t fix_size_bytes = 0;
public:
    PacketSections get_type() const { return PacketSections::Data;}
};

struct Other : public Section {
    uint32_t size_bytes = 0;
    virtual bool is_right() { return false;}
public:
    PacketSections get_type() const { return PacketSections::Data;}
};

enum class PacketErrors{
    //TODO(HP): better way is throw exception and in what explain error (developer dont need to implement these codes)
    Wrong_CRC,
    Wrong_Footer,
    Wrong_CMD,
    Wrong_Length_Include,

};

enum class PacketDefineErrors{
    PACKET_OK,
    CRC_Include_Data_But_Packet_Not,
    CRC_Include_Footer_But_Packet_Not,
    CRC_Include_Header_But_Packet_Not,
    CRC_Include_Length_But_Packet_Not,
    CRC_Include_CMD_But_Packet_Not,
    Length_Include_Data_But_Packet_Not,
    Length_Include_Footer_But_Packet_Not,
    Length_Include_Header_But_Packet_Not,
    Length_Include_CRC_But_Packet_Not,
    Length_Include_CMD_But_Packet_Not,
    Packet_Not_Include_Header,
    CMD_Factory_Null,
    CRC_Checker_Null
};

using Section_Shared = std::shared_ptr<Section>;

class AbstractPacketStructure {
public:
    virtual std::vector<std::shared_ptr<Section>> get_packet_structure() = 0;

    void analyze_packet_sections(const std::vector<std::shared_ptr<Section>>& packet_sections) {
        for (auto section : packet_sections) {
            auto type = section->get_type();
            switch (type) {
            case PacketSections::Header : {
                header_ = std::dynamic_pointer_cast<HeaderSection>(section);
                header_size_ = header_->content.size();
                is_header_exist_ = true;
                break;
            }
            case PacketSections::CMD : {
                cmd_ = std::dynamic_pointer_cast<CMDSection>(section);
                cmd_size_ = cmd_->size_bytes;
                is_cmd_exist_ = true;
                break;
            }
            case PacketSections::Length : {
                length_ = std::dynamic_pointer_cast<LengthSection>(section);
                length_size_ = length_->size_bytes;
                is_length_exist_ = true;
                break;
            }
            case PacketSections::Data : {
                data_ = std::dynamic_pointer_cast<DataSection>(section);
                data_size_ = data_->fix_size_bytes;
                is_data_exist_ = true;
                break;
            }
            case PacketSections::CRC : {
                crc_ = std::dynamic_pointer_cast<CRCSection>(section);
                crc_size_ = crc_->size_bytes;
                is_crc_exist_ = true;
                break;
            }
            case PacketSections::Footer : {
                footer_ = std::dynamic_pointer_cast<FooterSection>(section);
                footer_size_ = footer_->content.size();
                is_footer_exist_ = true;
                break;
            }
            case PacketSections::Other :{

                break;
            }
            }
        }
    }

    inline Section_Shared get_cmd()        const { return cmd_;    }
    inline Section_Shared get_crc()        const { return crc_;    }
    inline Section_Shared get_data()       const { return data_;   }
    inline std::shared_ptr<HeaderSection> get_header()     const { return header_; }
    inline Section_Shared get_length()     const { return length_; }
    inline Section_Shared get_footer()     const { return footer_; }

    inline bool is_cmd_exist()             const { return is_cmd_exist_; }
    inline bool is_crc_exist()             const { return is_cmd_exist_; }
    inline bool is_data_exist()            const { return is_data_exist_; }
    inline bool is_header_exist()          const { return is_header_exist_; }
    inline bool is_length_exist()          const { return is_length_exist_; }
    inline bool is_footer_exist()          const { return is_footer_exist_; }

    inline bool is_crc_include_cmd()       const { return is_crc_include_cmd_; }
    inline bool is_crc_include_data()      const { return is_crc_include_data_; }
    inline bool is_crc_include_header()    const { return is_crc_include_header_; }
    inline bool is_crc_include_length()    const { return is_crc_include_length_; }
    inline bool is_crc_include_footer()    const { return is_crc_include_footer_; }

    inline bool is_length_include_cmd()    const { return is_length_include_cmd_;    }
    inline bool is_length_include_crc()    const { return is_length_include_crc_;    }
    inline bool is_length_include_data()   const { return is_length_include_data_;   }
    inline bool is_length_include_header() const { return is_length_include_header_; }
    inline bool is_length_include_length() const { return is_length_include_length_; }
    inline bool is_length_include_footer() const { return is_length_include_footer_; }

    virtual ~AbstractPacketStructure(){}

private:
    bool check_crc_info (const std::shared_ptr<CRCSection>& crc) {
        PacketDefineErrors error = PacketDefineErrors::PACKET_OK;
            if (crc->include & PacketSections::Data && !is_data_exist_)
                error = PacketDefineErrors::CRC_Include_Data_But_Packet_Not;
            else if (crc->include & PacketSections::Footer && !is_footer_exist_)
                error = PacketDefineErrors::CRC_Include_Footer_But_Packet_Not;
            else if (crc->include & PacketSections::Header && !is_header_exist_)
                error = PacketDefineErrors::CRC_Include_Header_But_Packet_Not;
            else if (crc->include & PacketSections::CMD && !is_cmd_exist_)
                error = PacketDefineErrors::CRC_Include_CMD_But_Packet_Not;
            else if (crc->include & PacketSections::Length && !is_length_exist_)
                error = PacketDefineErrors::CRC_Include_Length_But_Packet_Not;
            else if (crc->crc_checker == nullptr)
               error = PacketDefineErrors::CMD_Factory_Null;
        return true;
    }
    bool is_cmd_ok() {

        return true;
    }
    bool check_length_info (/*LengthSection* length*/) {
        PacketDefineErrors error = PacketDefineErrors::PACKET_OK;
        if (length_->include & PacketSections::Data && !is_data_exist_)
            error = PacketDefineErrors::Length_Include_Data_But_Packet_Not;
        else if (length_->include & PacketSections::Footer && !is_footer_exist_)
            error = PacketDefineErrors::Length_Include_Footer_But_Packet_Not;
        else if (length_->include & PacketSections::Header && !is_header_exist_)
            error = PacketDefineErrors::Length_Include_Header_But_Packet_Not;
        else if (length_->include & PacketSections::CMD && !is_cmd_exist_)
            error = PacketDefineErrors::Length_Include_CMD_But_Packet_Not;
        else if (length_->include & PacketSections::CRC && !is_crc_exist_)
            error = PacketDefineErrors::Length_Include_CRC_But_Packet_Not;
        return true;
    }

    void set_crc_include_sections(const std::shared_ptr<CRCSection>& crc_) {
        if (crc_->include & PacketSections::Data)
            is_crc_include_data_ = true;
        if (crc_->include & PacketSections::CMD)
            is_crc_include_cmd_ = true;
        if (crc_->include & PacketSections::Length)
            is_crc_include_length_ = true;
        if (crc_->include & PacketSections::Footer)
            is_crc_include_footer_ = true;
        if (crc_->include & PacketSections::Header)
            is_crc_include_header_ = true;
    }


    uint32_t cmd_size_    = 0;
    uint32_t crc_size_    = 0;
    uint32_t data_size_   = 0;
    uint32_t header_size_ = 0;
    uint32_t length_size_ = 0;
    uint32_t footer_size_ = 0;

    bool is_cmd_exist_    = false;
    bool is_crc_exist_    = false;
    bool is_data_exist_   = false;
    bool is_header_exist_ = false;
    bool is_length_exist_ = false;
    bool is_footer_exist_ = false;

    bool is_crc_include_cmd_    = false;
    bool is_crc_include_data_   = false;
    bool is_crc_include_header_ = false;
    bool is_crc_include_length_ = false;
    bool is_crc_include_footer_ = false;

    bool is_length_include_cmd_    = false;
    bool is_length_include_crc_    = false;
    bool is_length_include_data_   = false;
    bool is_length_include_header_ = false;
    bool is_length_include_length_ = false;
    bool is_length_include_footer_ = false;

    std::shared_ptr<CMDSection> cmd_       = nullptr;
    std::shared_ptr<CRCSection> crc_       = nullptr;
    std::shared_ptr<DataSection> data_     = nullptr;
    std::shared_ptr<HeaderSection> header_ = nullptr;
    std::shared_ptr<LengthSection> length_ = nullptr;
    std::shared_ptr<FooterSection> footer_ = nullptr;
};

class AbstractMessageExtractor {
public:
    virtual std::shared_ptr<AbstractPacketStructure> get_next_message() = 0;
    virtual std::shared_ptr<AbstractPacketStructure> get_packet_structure() const =  0;
    virtual PacketDefineErrors get_packet_error() const = 0;
    virtual ~AbstractMessageExtractor() {}

protected:
    AbstractPacketStructure* extractor_;
};

} // namespace peripheral
} // namespace hp

#endif // TCPMESSAGEEXTRACTOR_H
