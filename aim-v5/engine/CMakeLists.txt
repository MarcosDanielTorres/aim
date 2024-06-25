cmake_minimum_required(VERSION 3.12)

project(Engine CXX C)

# Find Vulkan
find_package(Vulkan REQUIRED)

include(../CommonMacros.txt)

include_directories(../thirdparty/glfw-3.4/include)
include_directories(../thirdparty/imgui)
link_directories(../thirdparty/imgui)


file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c??)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

add_library(Engine SHARED ${SRC_FILES} ${HEADER_FILES})

set_property(TARGET Engine PROPERTY CXX_STANDARD 20)
set_property(TARGET Engine PROPERTY CXX_STANDARD_REQUIRED ON)


add_definitions(-DAIM_EXPORTS)

# Set the Vulkan SDK path
if (NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable is not set!")
endif()
set(Vulkan_INCLUDE_DIR $ENV{VULKAN_SDK}/Include)
include_directories(${Vulkan_INCLUDE_DIR})

target_link_libraries(Engine PUBLIC glfw Vulkan::Vulkan)

# This should be PUBLIC but it seams that cmake doesn't work when you dont build the .lib yourself
target_link_libraries(Engine PRIVATE imgui)
# So I do this and then in `Sandbox` I point to this imgui.lib



# https://cmake.org/cmake/help/v3.0/prop_tgt/ARCHIVE_OUTPUT_DIRECTORY.html?highlight=archive_output_directory

# For non-DLL platforms shared libraries are treated as library targets. (Linux, MacOS)
# For DLL platforms the DLL part of a shared library is treated as a runtime target and the corresponding import library is treated as an archive target. (Windows)
#   - DLL part (.dll, .pdb)
#   - Import library (.lib, .exp)

set_target_properties(Engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>"
)
