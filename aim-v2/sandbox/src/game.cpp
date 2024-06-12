#include "game.h"

bool game_init(game* game_inst) {
  INFO("Im in init function!");
  return true;
}

bool game_update(game* game_inst, float delta_time){
  INFO("Im in update function");
  return true;
}

bool game_render(game* game_inst, float delta_time){
  //INFO("Im in render function");
  return true;
}

bool game_resize(game* game_inst, int width, int height){
  //INFO("Im in resize function");
  return true;
}
