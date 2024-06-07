#include <iostream>
#include <game.h>
#include <entry.h>

game create_game() {
  game game_inst {.app_config {.name = "nananna", .width = 800, .height = 600 }};
  
  return game_inst;
}
