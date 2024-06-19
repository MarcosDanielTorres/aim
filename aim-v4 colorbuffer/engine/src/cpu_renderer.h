#pragma once
#include "defines.h"
#include "vk_types.h"
#include "vk_initializers.h"
#include <core/logger/logger.h>


struct render_config {
  int width;
  int height;
};

struct cpu_renderer {
    std::vector<uint32_t> pixels;
    uint32_t color;
    render_config config;
};

extern cpu_renderer* cpu_rend;

void init_cpu_renderer(int width, int height);
void cpu_clear(int width, int height, int color = 0);
 void AIM_API cpu_set_draw_color(uint32_t color = 0);
void AIM_API cpu_draw_rect(int x, int y, int w, int h, uint32_t color = 0);
 void AIM_API cpu_draw_point(int x, int y, uint32_t color = 0);
//void cpu_submit(VkCommandBuffer cmd, AllocatedImage* draw_image, VmaAllocator* allocator);
