#pragma once
#include "defines.h"

struct app_config {
  const char* name;
  int x;
  int y;
  int width;
  int height;
};

struct game {
  app_config app_config;
  void* state;

  bool (*init) (struct game* game_inst);
  bool (*update) (struct game* game_inst, float delta_time);
  bool (*render) (struct game* game_inst, float delta_time);
  bool (*resize) (struct game* game_inst, int width, int height);
};
