cmake_minimum_required(VERSION 3.12)

project(sandbox)

include(../CommonMacros.txt)

SETUP_APP(Sandbox "")
include_directories(../engine)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

#target_link_libraries(Sandbox PUBLIC Engine "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.lib")
target_link_libraries(Sandbox PUBLIC Engine)

set_target_properties(Sandbox PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>"
)