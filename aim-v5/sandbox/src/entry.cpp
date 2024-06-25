#include <../engine/src/entry.h>
#include "game.h"

bool create_game(game* game_inst) {
  game_inst->app_config.name = "Almost done!";
  game_inst->app_config.width = 1700;
  game_inst->app_config.height = 900;
  game_inst->init = game_init;
  game_inst->render = game_render;
  game_inst->update = game_update;
  game_inst->resize = game_resize;
  game_inst->on_gui_render = on_gui_render;
  return true;
}
