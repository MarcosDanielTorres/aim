#pragma once
#include <iostream>
#include "game.h"
#include "application.h"


extern game create_game();

int main() {
  game game_inst = create_game();

  HelloTriangleApplication app;

  app.run(&game_inst);
}
