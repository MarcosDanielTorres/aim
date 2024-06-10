#include <entry.h>
#include "game.h"

bool create_game(game* game_inst) {
  game_inst->app_config.name = "Conchudos";
  game_inst->app_config.width = 800;
  game_inst->app_config.height = 600;
  game_inst->init = game_init;
  game_inst->render = game_render;
  game_inst->update = game_update;
  game_inst->resize = game_resize;
  return true;
}
