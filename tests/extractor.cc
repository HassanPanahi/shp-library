#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "../buffer/bounded_buffer.h"
#include "message_extractor.h"

using namespace shp::network;

enum MessagesList {
    SystemInfo = 0x12,
    Status = 0x61
};

class SystemInfoClass : public AbstractSerializableMessage
{
public:
    void serialize(char *buffer, size_t size) {
        memcpy(&current_info_, buffer, sizeof (current_info_));
    }

    void deserialize(const char *buffer, size_t size) {

    }

    size_t get_serialize_size() const {
        return 3;
    }

    int get_type() const {return static_cast<int>(MessagesList::SystemInfo);}

private:
    struct Info{
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
    }current_info_;
};

class StatusClass : public AbstractSerializableMessage
{
public:
    void serialize(char *buffer, size_t size) {
        memcpy(&current_status_, buffer, sizeof (current_status_));
    }

    void deserialize(const char *buffer, size_t size) {

    }

    size_t get_serialize_size() const {
        return 1;
    }

    int get_type() const {return static_cast<int>(MessagesList::Status);}

private:
    struct StatusParams{
        int8_t temp;
    }current_status_;
};



class CMDFactory : public AbstractMessageFactory {
public:
    std::shared_ptr<AbstractSerializableMessage> build_message(const std::vector<uint8_t> cmd_data) {
        auto cmd = static_cast<MessagesList>(cmd_data[0]);
        if (cmd == MessagesList::SystemInfo) {
            return std::make_shared<SystemInfoClass>();
        } else if (cmd == MessagesList::Status){
            return std::make_shared<StatusClass>();
        } else {
            std::cout << "This command doesn't support" << std::endl;
            return nullptr;
        }
    }
};


void buffer_test()
{
    BoundedBuffer<uint8_t> buffer(6);

    std::vector<uint8_t> values(6);
    values[0] = 0xff;
    values[1] = 0xbb;
    values[2] = 0x12;
    values[3] = 0x0a;
    values[4] = 0x0b;
    values[5] = 0xff;

    buffer.write(values);

    std::vector<Section_Shared> sections;
    auto header = std::make_shared<HeaderSection>(std::vector<uint8_t>{0xff, 0xbb});
    sections.push_back(header);

    auto cmd_factory = std::make_shared<CMDFactory>();
    auto cmd = std::make_shared<CMDSection>(cmd_factory, 1);
    sections.push_back(cmd);

    MessageExtractor extrator(sections);
    extrator.write_bytes(values.data(), values.size());
    while (1) {
        auto mesg = extrator.get_next_message();
        std::cout << "message status: " << static_cast<int>(mesg.error) <<  std::endl;
    }

}

TEST(MemoryLeak, Constructor) {



    ASSERT_TRUE(1 == 1);
}


int main(int argc, char** argv)
{
    buffer_test();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
