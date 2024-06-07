#pragma once
#include "defines.h"

struct app_config {
  const char* name;
  int x;
  int y;
  int widht;
  int height;
};

struct game {
  app_config app_config;
  void* state;

  //bool (*init)(game* game);
  //bool (*update)(game* game, float delta_time);
  //bool (*render)(game* game, float delta_time);
  //bool (*resize)(game* game, int width, int height);

  bool init();
  bool update(float delta_time);
  bool render(float delta_time);
  bool resize(int width, int height);
};
