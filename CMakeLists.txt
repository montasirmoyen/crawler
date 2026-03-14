cmake_minimum_required(VERSION 3.10)
project(CppCrawler)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)

include_directories(src)

find_package(Threads REQUIRED)

add_executable(crawler 
    src/main.cpp 
)

target_link_libraries(crawler CURL::libcurl Threads::Threads)