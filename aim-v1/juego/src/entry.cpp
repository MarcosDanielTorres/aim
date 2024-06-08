#include <iostream>
#include "game.h"
#include <entry.h>

//game create_game() {
//  game game_inst {.app_config {.name = "Putos", .width = 800, .height = 600 }};
//  
//  return game_inst;
//}

bool create_game(game* game_inst) {
  game_inst->app_config.name = "Testbed";
  game_inst->app_config.width = 1280;
  game_inst->app_config.height = 720;
  game_inst->update = update;
  game_inst->render = render;
  game_inst->initialize = initialize;
  game_inst->on_resize = resize;

  
  return true;
}
