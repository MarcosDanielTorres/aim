#include<SDL2/SDL.h>


void pikuma_init() {
  	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		return;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	
	int windowWidth = 800;
	int windowHeight = 600;

	SDL_Window* window = SDL_CreateWindow(
		"Pikuma Window",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_RESIZABLE
	);
	if (!window) {
		return;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		return;
	}

	while(true){
	  SDL_SetRenderDrawColor(renderer, 255, 21, 21, 255);
	  SDL_RenderClear(renderer);
	  SDL_RenderPresent(renderer);
    }
}

void vkguide_init() {
    SDL_Init(SDL_INIT_VIDEO);

   SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

   SDL_Window* window = SDL_CreateWindow(
       "vkguide Window",
       SDL_WINDOWPOS_UNDEFINED,
       SDL_WINDOWPOS_UNDEFINED,
       800,
       600,
       window_flags);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		return;
	}

	while(true){
	  SDL_SetRenderDrawColor(renderer, 255, 21, 21, 255);
	  SDL_RenderClear(renderer);
	  SDL_RenderPresent(renderer);
    }

}

int main(int argc, char* argv[]){
  //pikuma_init();
  //vkguide_init();

  (void) argc;
  (void) argv;
  
    SDL_Init(SDL_INIT_VIDEO);

   SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

   SDL_Window* window = SDL_CreateWindow(
       "vkguide Window",
       SDL_WINDOWPOS_UNDEFINED,
       SDL_WINDOWPOS_UNDEFINED,
       800,
       600,
       window_flags);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		return -1;
	}

    // Main render loop
    while (true) {
        SDL_SetRenderDrawColor(renderer, 255, 21, 21, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        // Event handling
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }
    }


    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  return 0;
}

