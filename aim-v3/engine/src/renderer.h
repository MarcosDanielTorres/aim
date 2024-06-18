#pragma once
#include <iostream>


#include "vk_types.h"
#include "vk_pipelines.h"
#include "descriptors.h"

#include "VkBootstrap.h"
#include "vk_initializers.h"

#include "defines.h"
#include "game_types.h"
#include <core/logger/logger.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL.h>
#include <SDL_vulkan.h>

constexpr unsigned int FRAME_OVERLAP = 2;

struct deletion_queue {
  VkDevice device;
  VmaAllocator allocator;

  std::vector<VkImageView> imgs_views;
  std::vector<std::tuple<VkImage, VmaAllocation>> imgs;

  std::vector<DescriptorAllocator> descriptors_allocators;
  std::vector<VkDescriptorSetLayout> descriptor_sets_layouts;

  std::vector<VkCommandPool> command_pools;
  std::vector<VkDescriptorPool> descriptor_pools;
  std::vector<VkFence> fences;
  
  std::vector<VkPipelineLayout> pipelines_layouts;
  std::vector<VkPipeline> pipelines;

  std::vector<VmaAllocator> allocators;


  void push_imgs(VkImage img, VmaAllocation allocation) {
    imgs.push_back(std::make_tuple(img, allocation));
  }

  void push_imgs_views(VkImageView img_view) {
    imgs_views.push_back(img_view);
  }

  void push_descriptors_allocators(DescriptorAllocator da) {
    descriptors_allocators.push_back(da);
  }

  void push_descriptor_sets_layouts(VkDescriptorSetLayout ds) {
    descriptor_sets_layouts.push_back(ds);
  }

  void push_command_pools(VkCommandPool cp) {
    command_pools.push_back(cp);
  }

  void push_descriptor_pools(VkDescriptorPool dp) {
    descriptor_pools.push_back(dp);
  }

  void push_fences(VkFence fence) {
    fences.push_back(fence);
  }


  void push_pipelines_layouts(VkPipelineLayout pl) {
    pipelines_layouts.push_back(pl);
  }

  void push_pipelines(VkPipeline p) {
    pipelines.push_back(p);
  }

  void push_allocator(VmaAllocator allocator) {
    allocators.push_back(allocator);
  }


  void flush() {
    for (VkImageView img_view: imgs_views) {
      vkDestroyImageView(device, img_view, nullptr);
    }

    for (auto [img, allocation]: imgs) {
      vmaDestroyImage(allocator, img, allocation);
    }

    for (DescriptorAllocator ga: descriptors_allocators) {
      ga.destroy_pool(device);
    }

    for (VkDescriptorSetLayout ds: descriptor_sets_layouts) {
      vkDestroyDescriptorSetLayout(device, ds, nullptr);
    }

    for (VkCommandPool cp: command_pools) {
    }

    for (VkDescriptorPool dp: descriptor_pools) {
      vkDestroyDescriptorPool(device, dp, nullptr);
    }

    for (VkFence fence: fences) {
    }

    for (VkPipelineLayout pl: pipelines_layouts) {
      vkDestroyPipelineLayout(device, pl, nullptr);
    }

    for (VkPipeline p: pipelines) {
      vkDestroyPipeline(device, p, nullptr);
    }

    for (VmaAllocator allocator: allocators) {
      vmaDestroyAllocator(allocator);
    }

    imgs_views.clear();
    imgs.clear();
    descriptors_allocators.clear();
    descriptor_sets_layouts.clear();
    pipelines_layouts.clear();
    pipelines.clear();
    allocators.clear();
  }
};


struct frame_data {
  VkCommandPool command_pool;
  VkCommandBuffer main_command_buffer;
  VkSemaphore swapchain_semaphore, render_semaphore;
  VkFence render_fence;

  deletion_queue frame_deletion_queue;
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

  // allocator
  VmaAllocator allocator;
  // allocator

  // draw resources
  AllocatedImage draw_image;
  VkExtent2D draw_extent;
  float renderScale = 1.0f;
  // draw resources

  // descriptors
  DescriptorAllocator globalDescriptorAllocator;
  VkDescriptorSet _drawImageDescriptors;
  VkDescriptorSetLayout _drawImageDescriptorLayout;
  // descriptors

  // pipelines
  VkPipeline _gradientPipeline;
  VkPipelineLayout _gradientPipelineLayout;
  // pipelines

  VkExtent2D window_extent;

  // queues
  VkQueue graphics_queue;
  uint32_t graphics_queue_family;
  // queues
 
  // frame data
  frame_data frames[FRAME_OVERLAP];
  int frame_number;
  // frame data

  // deletion queue
  deletion_queue main_deletion_queue;
  // deletion queue

  // immediate submit
  VkDescriptorPool imguiPool;
  VkFence _immFence;
  VkCommandBuffer _immCommandBuffer;
  VkCommandPool _immCommandPool;
  // solo un commandBuffer de tipo main puede submitear a una queue
  // se pueden paralelizar los comandos pero solo uno puede submittear a la vez.
  // immediate submit

  bool resize_requested;


  frame_data& get_current_frame() { return frames[frame_number % FRAME_OVERLAP]; };
  void init(SDL_Window* window);
  void init_vulkan(SDL_Window* window);
  void init_swapchain();
  void init_pipelines();
  void init_background_pipelines();
  void create_swapchain(int width, int height);
  void destroy_swapchain();
  void init_commands();
  void init_sync_structures();
  void init_descriptors();
  void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
  void init_imgui(SDL_Window* window);
  void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);
  void render();
  void draw_background(VkCommandBuffer cmd);
  void cleanup();
  void resize_swapchain(SDL_Window* window);
};
