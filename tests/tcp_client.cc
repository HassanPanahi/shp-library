#include <iostream>
#include <vector>

#include <gtest/gtest.h>
#include <thread>

#include "../ip/tcp_client.h"

void tcp_client()
{
    auto tcp_client = std::make_shared<shp::network::TCPClient>("0.0.0.0", 8585);
    tcp_client->start();
    tcp_client->send("salam", 5);
    while(1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char** argv)
{
    tcp_client();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
