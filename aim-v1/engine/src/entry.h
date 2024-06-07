#pragma once
#include <iostream>
#include "game.h"


extern game create_game();

int main() {
  game game_inst = create_game();

  while(true) {
    printf("%s\n", game_inst.app_config.name);
    game_inst.resize(4, 5);
    printf("322\n");
  }
}
