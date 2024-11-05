cmake_minimum_required(VERSION 3.20)


project(Connect-4-game LANGUAGES CXX)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Protobuf REQUIRED)
find_package(SQLite3 REQUIRED)


enable_testing()

add_compile_definitions(ASIO_STANDALONE)

include_directories(third_party/websocketpp)


include_directories(src)
add_subdirectory(src)

install(TARGETS RUNTIME DESTINATION "${CMAKE_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}")




