cmake_minimum_required(VERSION 3.12)
project("hw" LANGUAGES C CXX VERSION "0.1")

add_executable(main ./main.cpp ./tests.cpp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

TARGET_LINK_LIBRARIES(main ${Boost_LIBRARIES} rt)