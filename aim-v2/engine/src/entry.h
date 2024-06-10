#pragma once
#include <iostream>
#include "game_types.h"
#define SDL_MAIN_HANDLED
//#include <vulkan/vulkan.h>
//#include <SDL2/SDL.h>
#include "app.h"

extern bool create_game(game* game_inst);

int main(int argc, char* argv[]) {
  //   SDL_Init(SDL_INIT_VIDEO);
  //
  //   SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
  //
  //   SDL_Window* window = SDL_CreateWindow(
  //       "LA PUTA MADRE",
  //       SDL_WINDOWPOS_UNDEFINED,
  //       SDL_WINDOWPOS_UNDEFINED,
  //       800,
  //       600,
  //       window_flags);
  //
  //	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
  //	if (!renderer) {
  //		return -1;
  //	}
	game game_inst;
	create_game(&game_inst);
  game_inst.init(&game_inst);
  game_inst.update(&game_inst, 32.0f);

  app_init(game_inst.app_config);
  app_run(game_inst);
	SDL_Quit();
  return 0;
}
