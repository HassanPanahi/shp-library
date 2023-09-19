cmake_minimum_required(VERSION 3.5)

project(shp LANGUAGES CXX)
find_package(Boost COMPONENTS system thread)
find_package(Threads)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file (GLOB Srcs message_extractor.h message_extractor.cc serial/*.h serial/*.cc ip/*.h ip/*.cc ip/*.hpp  buffer/bounded_buffer.h buffer/abstract_buffer.h )


add_library(${PROJECT_NAME} ${Srcs})
#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wsign-conversion -Werror)
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES}  ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)


add_subdirectory(tests)


