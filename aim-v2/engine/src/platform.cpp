#include "platform.h"
//#include <ctime>
#include <chrono>
#include <thread>

int platform_state::init(app_config config) {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  SDL_Window* window = SDL_CreateWindow(
    config.name,
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    config.width,
    config.height,
    window_flags);

  // Initialize platform_state members
  state.window = window;
    
  INFO("SDL Window intialized!");
  renderer_inst.init(state.window);

  INFO("SDL Window intialized!");
  return 0;
}

void platform_state::run(bool* is_running) {
  SDL_Event e;
  bool bQuit = false;
  bool stop_rendering = false;

  while (!bQuit){
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
	bQuit = true;
      }

      if (e.type == SDL_WINDOWEVENT) {
	if (e.type == SDL_WINDOWEVENT_MINIMIZED) {
	  stop_rendering = true;
	}

	if (e.type == SDL_WINDOWEVENT_MINIMIZED) {
	  stop_rendering = false;
	}
      }
    }
    if (stop_rendering) {
      // throttle the speed to avoid the endless spinning
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    renderer_inst.render();
  }
}

void platform_state::cleanup(){
  renderer_inst.cleanup();
  SDL_DestroyWindow(state.window);
}
