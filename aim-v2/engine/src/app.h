#pragma once
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include "VkBootstrap.h"
#include "defines.h"
#include "game_types.h"
//#include "renderer.h"
#include "platform.h"


struct app_state {
  int width;
  int height;
  bool is_running;
  bool is_suspended;

  platform_state plat_state;

  int AIM_API init(app_config config);
  void AIM_API run(game game_inst);
  void AIM_API cleanup();
};

