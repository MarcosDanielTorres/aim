#pragma once
#include <../engine/src/game_types.h>
#include <../engine/src/core/logger/logger.h>

#include <imgui.h>

bool game_init(game* game_inst);
bool game_update(game* game_inst, float delta_time);
bool game_render(game* game_inst, float delta_time);
bool game_resize(game* game_inst, int width, int height);
bool on_gui_render(game* game_inst);
