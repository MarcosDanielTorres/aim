#include <iostream>
#include <game.h>
#include <entry.h>

game create_game() {
  game game_inst {.app_config {.name = "nananna" }};
  
  return game_inst;
}
