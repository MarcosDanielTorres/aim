find_package(Vulkan REQUIRED)
add_library(imgui STATIC)

target_include_directories(imgui PUBLIC .)

target_sources(imgui PRIVATE 
  imgui.h
  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_widgets.cpp
  imgui_tables.cpp
  backends/imgui_impl_vulkan.cpp
  backends/imgui_impl_opengl3.cpp
  backends/imgui_impl_glfw.cpp
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan)