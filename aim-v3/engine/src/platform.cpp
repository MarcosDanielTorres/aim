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
      ImGui_ImplSDL2_ProcessEvent(&e);
    }
    if (stop_rendering) {
      // throttle the speed to avoid the endless spinning
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }

    if (renderer_inst.resize_requested) {
      renderer_inst.resize_swapchain(state.window);
    }


    // imgui
    // imgui new frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    if (ImGui::Begin("background")) {
      ImGui::SliderFloat("Render Scale",&renderer_inst.renderScale, 0.3f, 1.f);
      //other code
    			ImGui::End();
		}
		else {
			ImGui::End();

		}

    
    ////some imgui UI to test
    ImGui::ShowDemoWindow();
    
    ////make imgui calculate internal draw structures
    ImGui::Render();
    // imgui

    renderer_inst.render();
  }
}

void platform_state::cleanup(){
  renderer_inst.cleanup();
  SDL_DestroyWindow(state.window);
}
