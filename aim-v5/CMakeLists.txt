cmake_minimum_required(VERSION 3.12)

project(aim CXX C)


set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")


add_subdirectory(thirdparty/glfw-3.4)
include_directories(thirdparty/glfw-3.4/include)
add_compile_definitions("${CMAKE_PROJET_NAME}" PUBLIC AIM_ENGINE_ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/engine/assets/")


add_subdirectory(thirdparty/imgui-1.90.8)
include_directories(thirdparty/imgui-1.90.8)

include_directories(thirdparty/stb_image)



set(PHYSICS_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/JoltPhysics)
include(${PHYSICS_REPO_ROOT}/Jolt/Jolt.cmake)


add_subdirectory(engine)
add_subdirectory(sandbox)