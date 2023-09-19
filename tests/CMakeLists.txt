cmake_minimum_required(VERSION 3.5)

project(tests LANGUAGES CXX)
include_directories(..)
set(CMAKE_BUILD_TYPE Debug)

add_executable(buffer buffer.cc)
target_link_libraries(buffer shp gtest)

add_executable(tcp-client tcp_client.cc)
target_link_libraries(tcp-client shp gtest)

add_executable(extractor extractor.cc)
target_link_libraries(extractor shp gtest)
