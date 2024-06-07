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

  bool init();
  bool update(float delta_time);
  bool render(float delta_time);
  bool resize(int width, int height);
};
