
#include <../engine/src/game_types.h>
#include <../engine/src/core/logger/logger.h>

//bool game::init(){
//  printf("Im in init\n");
//  return true;
//}
//
//bool game::update(float delta_time){
//  printf("Im in update\n");
//  return true;
//}
//
//bool game::render(float delta_time){
//  printf("Im in render\n");
//  return true;
//}
//
//bool game::resize(int width, int height){
//  printf("Im in resize: %d, %d\n", width, height);
//  return true;
//}



bool initialize(game* game_inst){
  printf("Im in init\n");
  return true;
}

bool update(game* game_inst, float delta_time){
  INFO("Im in update %f\n", delta_time);
  return true;
}

bool render(game* game_inst, float delta_time){
  printf("Im in render\n");
  return true;
}

bool resize(game* game_inst, int width, int height){
  printf("Im in resize: %d, %d\n", width, height);
  return true;
}

