set (CMAKE_VERBOSE_MAKEFILE ON)

# Why isn't this inherited from top-level?
include_directories(/usr/local/include /usr/local/include/foonathan_memory)

add_executable(ev3sensorstest ev3test_main.cpp ev3sensorsubscriber.cpp ${generated_sources})
target_link_libraries(ev3sensorstest fastrtps fastcdr foonathan_memory ev3dev)
