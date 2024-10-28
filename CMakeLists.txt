cmake_minimum_required(VERSION 3.20)


project(OneApiInfrastructure LANGUAGES CXX)



#set(CLANG_COMPILE_OPTIONS -O3 -flto)
#add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${CLANG_COMPILE_OPTIONS}>")

#add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-O3>")
#add_link_options("$<$<COMPILE_LANGUAGE:CXX>:-flto>")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Protobuf REQUIRED)


enable_testing()

include_directories(src)
add_subdirectory(src)

install(TARGETS RUNTIME DESTINATION "${CMAKE_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}")




