cmake_minimum_required(VERSION 3.14)

project(ev3ddsagent)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_VERBOSE_MAKEFILE ON)

list(APPEND CMAKE_PREFIX_PATH "/opt/ev3dds")
link_directories(AFTER "/opt/ev3dds/lib")

find_package(fastcdr REQUIRED)
find_package(foonathan_memory REQUIRED)
find_package(fastrtps REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fcompare-debug-second)

set (generated_sources ${CMAKE_CURRENT_BINARY_DIR}/ev3data.cxx ${CMAKE_CURRENT_BINARY_DIR}/ev3dataPubSubTypes.cxx)

add_custom_command(
    OUTPUT  ${generated_sources}
    COMMAND fastddsgen -cs -d ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ev3data.idl
    DEPENDS ev3data.idl   # Treated as relative to CMAKE_CURRENT_SOURCE_DIR
)

add_executable(ev3ddsnode ev3dds_main.cpp ev3nodepublisher.cpp ${generated_sources})
target_link_libraries(ev3ddsnode fastrtps fastcdr foonathan_memory ev3dev)

add_subdirectory(test)
