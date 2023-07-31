#include <iostream>
#include "bounded_buffer.h"
#include <vector>
#include "tcpserver.h"

void buffer_test()
{
    BoundedBuffer<int> buffer(50);

    buffer.write(10);

    std::vector<int> nums{4, 3};
    buffer.write(nums);


    const uint32_t buf_size = 20;
    int a[buf_size];
    for (uint32_t i = 0; i < buf_size; i++)
        a[i] = 100 + i;
    buffer.write(a);

    std::cout << "buf size: " << buffer.size() << std::endl;
    int * new_nums = new int[buf_size];
    for (uint32_t i = 0; i < buf_size; i++)
        new_nums[i] = 200 + i;
    buffer.write(new_nums, 10);
    auto new_buffer = buffer;
    int counter = new_buffer.size();
    for (uint32_t i = 0 ; i < counter; i++)
        std::cout <<  new_buffer.read() << std::endl;


    BoundedBuffer<int> buffer1(10);
    BoundedBuffer<int> buffer2(buffer1); // Copy constructor is called here.
    BoundedBuffer<int> buffer3 = buffer1;


}

void tcp_server_test()
{
    TCPServer server(8585);
    server.start();
    while(1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
//    buffer_test();
    tcp_server_test();
    return 0;
}
