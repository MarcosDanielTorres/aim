#pragma once
#include <iostream>

#include <vector>

#include "VkBootstrap.h"
#include "vk_initializers.h"
#include "defines.h"
#include "game_types.h"
#include <core/logger/logger.h>
#include <SDL2/SDL.h>
#include <SDL_vulkan.h>

constexpr unsigned int FRAME_OVERLAP = 2;

struct frame_data {
  VkCommandPool command_pool;
  VkCommandBuffer main_command_buffer;
  VkSemaphore swapchain_semaphore, render_semaphore;
  VkFence render_fence;
};


struct renderer {
  // context
  VkInstance instance;
  VkDebugUtilsMessengerEXT debug_messenger;
  VkPhysicalDevice physical_device;
  VkDevice device;
  VkSurfaceKHR surface;
  // context

  // swapchain
  VkSwapchainKHR swapchain;
  VkFormat swapchain_img_fmt;
  std::vector<VkImage> swapchain_imgs;
  std::vector<VkImageView> swapchain_imgs_views;
  VkExtent2D swapchain_extent;
  // swapchain


  VkExtent2D window_extent;

  VkQueue graphics_queue;
  uint32_t graphics_queue_family;
 
  frame_data frames[FRAME_OVERLAP];
  int frame_number;

  frame_data& get_current_frame() { return frames[frame_number % FRAME_OVERLAP]; };
  void init(SDL_Window* window);
  void init_vulkan(SDL_Window* window);
  void init_swapchain();
  void create_swapchain(int width, int height);
  void destroy_swapchain();
  void init_commands();
  void init_sync_structures();
  void render();
  void cleanup();
};
