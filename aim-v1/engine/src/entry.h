#pragma once
#include <iostream>
#include "game_types.h"
#include "application.h"


//extern game create_game();
extern bool create_game(game* game_inst);

int main() {
  //game game_inst = create_game();
  //game_inst.update(1.0f);


  game game_inst;
  create_game(&game_inst);

  HelloTriangleApplication app;

  app.run(&game_inst);
}
