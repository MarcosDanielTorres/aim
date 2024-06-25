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

  //bool init();
  //bool update(float delta_time);
  //bool render(float delta_time);
  //bool resize(int width, int height);

  bool (*initialize) (struct game* game_inst);
  bool (*update) (struct game* game_inst, float delta_time);
  bool (*render) (struct game* game_inst, float delta_time);
  bool (*on_resize) (struct game* game_inst, int width, int height);

  //bool (*init)(game* game_inst);
  //bool (*update)(game* game_inst, float delta_time);
  //bool (*render)(game* game_inst, float delta_time);
  //bool (*resize)(game* game_inst, int width, int height);
};
