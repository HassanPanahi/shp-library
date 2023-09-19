#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "../buffer/bounded_buffer.h"
#include "message_extractor.h"

using namespace shp::network;

void buffer_test()
{
    BoundedBuffer<uint8_t> buffer(6);

    std::vector<uint8_t> values(6);
    values[0] = 0xff;
    values[1] = 0xbb;
    values[2] = 0x01;
    values[3] = 0x0a;
    values[4] = 0x0b;
    values[5] = 0xff;

    buffer.write(values);

    std::vector<Section_Shared> sections;
    auto header = std::make_shared<HeaderSection>(std::vector<uint8_t>{0xff, 0xbb});
    sections.push_back(header);

    MessageExtractor extrator(sections);
    extrator.write_bytes(values.data(), values.size());
    auto mesg = extrator.get_next_message();
    std::cout << "finished" << std::endl;

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
