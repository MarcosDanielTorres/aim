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



set(PHYSICS_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/JoltPhysics-5.0.0)
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE ON)
set(DOUBLE_PRECISION OFF)
set(GENERATE_DEBUG_SYMBOLS ON)
set(CROSS_PLATFORM_DETERMINISTIC ON)
set(FLOATING_POINT_EXCEPTIONS_ENABLED OFF)
set(OBJECT_LAYER_BITS 32)
set(USE_SSE4_1 ON)
set(USE_SSE4_2 ON)
set(USE_AVX ON)
set(USE_AVX2 ON)
set(USE_AVX512 OFF)
set(USE_LZCNT ON)
set(USE_TZCNT ON)
set(USE_F16C ON)
set(USE_FMADD ON)
# set(BUILD_SHARED_LIBS ON)
set(INTERPROCEDURAL_OPTIMIZATION ON)
include(${PHYSICS_REPO_ROOT}/Jolt/Jolt.cmake)

#target_compile_definitions(Jolt PUBLIC "JPH_DEBUG_RENDERER")


include_directories(thirdparty/tinygltf-2.9.2)

add_subdirectory(thirdparty/assimp-5.2.5)
include_directories(thirdparty/assimp-5.2.5/include)

add_subdirectory(engine)
#add_subdirectory(sandbox)