#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "../buffer/bounded_buffer.h"

void buffer_test()
{
    BoundedBuffer<int> buffer(50);
    buffer.write(10);

    std::vector<int> nums{4, 3};
    buffer.write(nums);


    std::array<int, 3> nums3{85, 85};
    buffer.write(nums3);

    const uint32_t buf_size = 20;
    int a[buf_size];
    for (uint32_t i = 0; i < buf_size; i++)
        a[i] = static_cast<int>(100 + i);
    buffer.write(a);

    std::cout << "buf size: " << buffer.size() << std::endl;
    int * new_nums = new int[buf_size];
    for (uint32_t i = 0; i < buf_size; i++)
        new_nums[i] = static_cast<int>(200 + i);
    buffer.write(new_nums, 10);
    auto new_buffer = buffer;
    uint32_t counter = static_cast<uint32_t>(new_buffer.size());
    for (uint32_t i = 0 ; i < counter; i++)
        std::cout <<  new_buffer.read() << std::endl;
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
