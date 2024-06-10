#pragma once
#include <vulkan/vulkan.h>
#include "defines.h"
#include "platform.h"
#include "game_types.h"

struct renderer {
  int frame_number;
  VkExtent2D window_extent;

  void init();
  void clean_up();
  void render();
  void run();
};

struct app_state {
  int width;
  int height;
  bool is_running;
  bool is_suspended;
};

int AIM_API app_init(app_config config);
void AIM_API app_run(game game_inst);
