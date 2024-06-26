cmake_minimum_required(VERSION 3.12)

project(aim CXX C)


set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")


add_subdirectory(thirdparty/glfw-3.4)
include_directories(thirdparty/glfw-3.4/include)


add_subdirectory(thirdparty)
include_directories(thirdparty/imgui)

add_subdirectory(engine)
add_subdirectory(sandbox)