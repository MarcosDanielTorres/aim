cmake_minimum_required(VERSION 3.12)

project(sandbox)

include(../CommonMacros.txt)

SETUP_APP(Sandbox "")
include_directories(../engine)

target_link_libraries(Sandbox PRIVATE Engine)

set_target_properties(Sandbox PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>"
)