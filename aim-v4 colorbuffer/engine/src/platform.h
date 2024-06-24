#pragma once

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include <SDL2/SDL.h>
#include <SDL_vulkan.h>

#include <core/logger/logger.h>

#include "game_types.h"
#include "renderer.h"

//// Exported function to create and retrieve the ImGui context
extern "C" AIM_API ImGuiContext* GetImGuiContext();
//
//// Exported function to set the current ImGui context
//extern "C" IMGUI_API void SetImGuiContext(ImGuiContext* context) {
//    ImGui::SetCurrentContext(context);
//}

struct platform_internal_state {
  SDL_Window* window;
};

struct platform_state {
  platform_internal_state state;
  renderer renderer_inst;
  int AIM_API init(app_config config);
  void AIM_API run(bool* is_running, game* game_inst);
  void AIM_API cleanup();
};
