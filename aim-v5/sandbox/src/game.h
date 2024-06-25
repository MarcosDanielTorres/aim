#pragma once
#include <../engine/src/game_types.h>

bool initialize(game* game_inst);
bool update(game* game_inst, float delta_time);
bool render(game* game_inst, float delta_time);
bool resize(game* game_inst, int width, int height);
