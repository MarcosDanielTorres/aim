#pragma once
#include <iostream>
#include "game_types.h"
#define SDL_MAIN_HANDLED
#include "app.h"

extern bool create_game(game* game_inst);

int main(int argc, char* argv[]) {
  game game_inst;
  create_game(&game_inst);

  app_config config = game_inst.app_config;

  app_state* app = (app_state*) malloc(sizeof(app_state));
  new (app) app_state {
    .width = config.width, .height = config.height,
    .is_running = true,
  };

  app->init(config);
  app->run(game_inst);
  app->cleanup();

  return 0;
}
