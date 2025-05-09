find_package(Vulkan REQUIRED)
add_library(imgui STATIC)

target_include_directories(imgui PUBLIC imgui)

target_sources(imgui PRIVATE 
  imgui/imgui.h
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_tables.cpp
  imgui/backends/imgui_impl_vulkan.cpp
  imgui/backends/imgui_impl_opengl3.cpp
  imgui/backends/imgui_impl_glfw.cpp
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan)