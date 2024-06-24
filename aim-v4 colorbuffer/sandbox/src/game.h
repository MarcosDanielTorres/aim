#pragma once
#include <game_types.h>
#include <core/logger/logger.h>
#include <cpu_renderer.h>

#include <imgui.h>

bool game_init(game* game_inst);
bool game_update(game* game_inst, float delta_time);
bool game_render(game* game_inst, float delta_time);
bool game_resize(game* game_inst, int width, int height);
bool on_gui_render(game* game_inst);
