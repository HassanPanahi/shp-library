cmake_minimum_required(VERSION 3.5)

project(shp LANGUAGES CXX)
find_package(Boost COMPONENTS serialization filesystem thread system regex)
find_package(Threads)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file (GLOB Srcs main.cc bounded_buffer.h abstract_buffer.h)

add_executable(${PROJECT_NAME} ${Srcs})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES}  ${CMAKE_THREAD_LIBS_INIT})

