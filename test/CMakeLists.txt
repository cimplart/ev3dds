set (CMAKE_VERBOSE_MAKEFILE ON)

# Why isn't this inherited from top-level?
include_directories(/opt/ev3dds/include /opt/ev3dds/include/foonathan_memory)

# Needed until cmake 3.20
set_source_files_properties(${generated_sources} PROPERTIES GENERATED TRUE)

if (${HOST_PC_BUILD})
add_definitions(-DREMOTE_PEER)

add_custom_command(
    OUTPUT  ${generated_sources}
    COMMAND fastddsgen -cs -d ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../ev3data.idl
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../ev3data.idl   # Treated as relative to CMAKE_CURRENT_SOURCE_DIR
)
endif()

add_executable(ev3sensorstest ev3test_main.cpp ev3sensorsubscriber.cpp ${generated_sources})
target_link_libraries(ev3sensorstest fastrtps fastcdr foonathan_memory)
