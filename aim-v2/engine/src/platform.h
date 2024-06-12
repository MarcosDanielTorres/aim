#pragma once

#include <SDL2/SDL.h>
#include <SDL_vulkan.h>

#include <core/logger/logger.h>

#include "game_types.h"
#include "renderer.h"

struct platform_internal_state {
  SDL_Window* window;
};

struct platform_state {
  platform_internal_state state;
  renderer renderer_inst;
  int AIM_API init(app_config config);
  void AIM_API run(bool* is_running);
  void AIM_API cleanup();
};
