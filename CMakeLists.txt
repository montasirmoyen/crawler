cmake_minimum_required(VERSION 3.10)
project(CppCrawler)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)

include_directories(src)

add_executable(crawler 
    src/main.cpp 
    src/core/url_queue.cpp 
    src/network/fetcher.cpp
)

target_link_libraries(crawler CURL::libcurl Threads::Threads)