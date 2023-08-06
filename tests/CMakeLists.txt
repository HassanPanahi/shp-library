cmake_minimum_required(VERSION 3.5)

project(tests LANGUAGES CXX)

add_executable(buffer buffer.cc)

include_directories(..)

target_link_libraries(buffer shp gtest)
