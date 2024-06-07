#include <game.h>

bool game::init(){
  printf("Im in init\n");
  return true;
}

bool game::update(float delta_time){
  printf("Im in update\n");
  return true;
}

bool game::render(float delta_time){
  printf("Im in render\n");
  return true;
}

bool game::resize(int width, int height){
  printf("Im in resize: %d, %d\n", width, height);
  return true;
}
