#include "platform.h"

static platform_state* plat_state = NULL;

int platform_init(app_config config) {
    SDL_Init(SDL_INIT_VIDEO);

   SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

   SDL_Window* window = SDL_CreateWindow(
       config.name,
       SDL_WINDOWPOS_UNDEFINED,
       SDL_WINDOWPOS_UNDEFINED,
       config.width,
       config.height,
       window_flags);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		return -1;
	}

	INFO("SDL Renderer initialized!");

   // Allocate memory for plat_state
    plat_state = (platform_state*)malloc(sizeof(platform_state));
    if (!plat_state) {
        fprintf(stderr, "Error allocating memory for plat_state\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Initialize plat_state members
    plat_state->state.window = window;
    plat_state->state.renderer = renderer;


	INFO("SDL Window intialized!");
	return 0;
}

void platform_run(game game_inst) {
     while(true){
	SDL_SetRenderDrawColor(plat_state->state.renderer, 255, 21, 21, 255);
	SDL_RenderClear(plat_state->state.renderer);
	SDL_RenderPresent(plat_state->state.renderer);
    }
  
}
