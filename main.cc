#include <iostream>
#include <vector>

#include "bounded_buffer.h"
//#include "tcpserver.h"
//#pragma GCC diagnostic error "-Wconversion"

void buffer_test()
{
    BoundedBuffer<int> buffer(50);

    buffer.write(10);

    std::vector<int> nums{4, 3};
    buffer.write(nums);

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

//void tcp_server_test()
//{
////    TCPServer server(8585);
////    server.start();
////    while(1) {
////        std::this_thread::sleep_for(std::chrono::seconds(1));
////    }
//}

int main()
{
    buffer_test();
//    tcp_server_test();
    return 0;
}
