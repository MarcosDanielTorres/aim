#include "app.h"
#include <core/logger/logger.h>

int app_init(app_config config) {
  return platform_init(config);
}

void app_run(game game_inst) {
  INFO("APP IS RUNNING");
  platform_run(game_inst);
}
