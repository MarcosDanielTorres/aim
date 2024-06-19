#include "app.h"
#include <core/logger/logger.h>

// these are valid
// app = (app_state*) malloc(sizeof(app_state));
// new (app) app_state {.width = config.width, .height = config.height};

// this is valid
// app = new app_state{.width = 100, .height = 322};

int app_state::init(app_config config) {
  platform_state plat_state2 {
    .renderer_inst {
      .window_extent {
	static_cast<uint32_t>(config.width), static_cast<uint32_t>(config.height)
      }
    }
  };
  plat_state2.init(config);
   
  plat_state = plat_state2;

  return 0;
}

void app_state::run(game game_inst) {
  INFO("APP IS RUNNING: %d %d, window_extent: %d  %d", width, height, plat_state.renderer_inst.window_extent, is_running);

  plat_state.run(&is_running, &game_inst);
}


void app_state::cleanup() {
  INFO("Cleaning up resources!"); 
  //  renderer_inst.cleanup();
  plat_state.cleanup();
}
