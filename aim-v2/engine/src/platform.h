#pragma once
#include <SDL2/SDL.h>
#include "game_types.h"
#include <core/logger/logger.h>

struct platform_internal_state {
  SDL_Window* window;
  SDL_Renderer* renderer;
};

struct platform_state {
  platform_internal_state state;
};

int AIM_API platform_init(app_config config);
void AIM_API platform_run(game game_inst);
