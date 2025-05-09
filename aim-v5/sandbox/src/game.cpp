#include "game.h"
//#include "../engine/src/platform.h"
#include "../engine/src/jolt_debug_renderer.h"
#include "../thirdparty/JoltPhysics-5.0.0/Jolt/Jolt.h"
#include "../thirdparty/JoltPhysics-5.0.0/Jolt/Renderer/DebugRenderer.h"

bool game_init(game* game_inst) {
  INFO("Im in init function!");
  return true;
}

bool game_update(game* game_inst, float delta_time){
  INFO("Im in update function");
  return true;
}

int posX = 0;

bool game_render(game* game_inst, float delta_time){
  //cpu_draw_rect(100, 300, 400, 400);
  //posX += delta_time;
  //cpu_set_draw_color(0xFF0000FF); // cpu_renderer.set_base_color();
  //cpu_draw_rect(100, 200, 400, 400);
  //cpu_draw_rect(400, 300, 40, 70, 0xFFFF0000);
  //cpu_draw_point(600 + posX, 300, 0xFF00FFFF);

  return true;
}

bool game_resize(game* game_inst, int width, int height){
  //INFO("Im in resize function");
  return true;
}

bool on_gui_render(game* game_inst){
  INFO("Im in on_gui_render!");
  // TODO: ponerlo en el init y llamar esto una sola vez
  //ImGui::SetCurrentContext(GetImGuiContext());

//  ImGui::ShowDemoWindow();
  ImGui::Begin("Settings");

  ImGui::Text("Renderer2D Stats:");
  ImGui::Text("Draw Calls: %d", 10);
  ImGui::Text("Quads: %d", 20);

  ImGui::End();

  return true;
}
