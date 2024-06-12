#include "renderer.h"


    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    const char* type;
    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            type = "GENERAL";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            type = "VALIDATION";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            type = "PERFORMANCE";
            break;
        default:
            type = "UNKNOWN";
            break;
    }
      
	     const char* severity;
	  switch (messageSeverity) {
	      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		  DEBUG("%s %s", type, pCallbackData->pMessage);
	          break;
	      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		  INFO("%s %s", type, pCallbackData->pMessage);
	          break;
	      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		  WARN("%s %s", type, pCallbackData->pMessage);
	          break;
	      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		  ERROR("%s %s", type, pCallbackData->pMessage);
	          break;
	      default:
	          severity = "UNKNOWN";
		  WARN("%s %s %s", severity, type, pCallbackData->pMessage);
	          break;
	  }
        return VK_FALSE;
    }

void renderer::init(SDL_Window* window){
  init_vulkan(window);
  init_swapchain();
  init_commands();

  init_sync_structures();
  //init_descriptors();
  //init_pipelines();
  //init_imgui();
  //init_default_data();
}

void renderer::init_vulkan(SDL_Window* window){
  vkb::InstanceBuilder builder;
  auto inst_ret = builder.set_app_name("Example Vulkan App")
    .request_validation_layers(true)
    .set_debug_messenger_severity(
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    )
    .set_debug_messenger_type(
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT 
    )
    .set_debug_callback(debug_callback)
    .require_api_version(1, 3, 0)
    .build();

  vkb::Instance vkb_inst = inst_ret.value();

  instance = vkb_inst.instance;
  debug_messenger = vkb_inst.debug_messenger;

  SDL_Vulkan_CreateSurface(window, instance, &surface);

  //vulkan 1.3 features
  VkPhysicalDeviceVulkan13Features features{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
  };
  features.dynamicRendering = true;
  features.synchronization2 = true;

  //vulkan 1.2 features
  VkPhysicalDeviceVulkan12Features features12{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
  };
  features12.bufferDeviceAddress = true;
  features12.descriptorIndexing = true;

  vkb::PhysicalDeviceSelector selector{ vkb_inst };
  vkb::PhysicalDevice physicaldevice = selector
    .set_minimum_version(1, 3)
    .set_required_features_13(features)
    .set_required_features_12(features12)
    .set_surface(surface)
    .select()
    .value();
  
  
  //create the final vulkan device
  vkb::DeviceBuilder deviceBuilder{ physicaldevice };
  
  vkb::Device vkbDevice = deviceBuilder.build().value();
  
  // Get the VkDevice handle used in the rest of a vulkan application
  device = vkbDevice.device;
  physical_device = physicaldevice.physical_device;

  graphics_queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  graphics_queue_family = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
  
  
  INFO("Init vulkan!");
}

void renderer::init_swapchain(){
  create_swapchain(window_extent.width, window_extent.height);
}

void renderer::init_commands() {
  VkCommandPoolCreateInfo command_pool_info = {};
  command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  command_pool_info.pNext = nullptr;
  command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  command_pool_info.queueFamilyIndex = graphics_queue_family;

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateCommandPool(device, &command_pool_info, nullptr, &frames[i].command_pool));
    VkCommandBufferAllocateInfo cmd_alloc_info = {};
    cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc_info.pNext = nullptr;
    cmd_alloc_info.commandPool = frames[i].command_pool;
    cmd_alloc_info.commandBufferCount = 1;
    cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(device, &cmd_alloc_info, &frames[i].main_command_buffer));
  }
}

void renderer::init_sync_structures() {
  VkFenceCreateInfo fence_create_info = {};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_create_info.pNext = nullptr;
  fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkSemaphoreCreateInfo semaphore_create_info = {};
  semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphore_create_info.pNext = nullptr;
  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateFence(device, &fence_create_info, nullptr, &frames[i].render_fence));

    VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &frames[i].swapchain_semaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &frames[i].render_semaphore));
  }
}




void renderer::create_swapchain(int width, int height){
  vkb::SwapchainBuilder swapchainBuilder { physical_device, device, surface };
  swapchain_img_fmt = VK_FORMAT_R8G8B8A8_UNORM;

  vkb::Swapchain vkbSwapchain = swapchainBuilder
    .set_desired_format(VkSurfaceFormatKHR { .format = swapchain_img_fmt, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    .set_desired_extent(width, height)
    .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    .build()
    .value();


  swapchain = vkbSwapchain.swapchain; 
  swapchain_extent = vkbSwapchain.extent; 
  swapchain_imgs = vkbSwapchain.get_images().value(); 
  swapchain_imgs_views = vkbSwapchain.get_image_views().value();
}


void renderer::render() {
  VK_CHECK(vkWaitForFences(device, 1, &get_current_frame().render_fence, true, 1000000000));
  VK_CHECK(vkResetFences(device, 1, &get_current_frame().render_fence));
  uint32_t swapchain_img_index;
  VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().swapchain_semaphore, nullptr, &swapchain_img_index));

  //naming it cmd for shorter writing
  VkCommandBuffer cmd = get_current_frame().main_command_buffer;

  // now that we are sure that the commands finished executing, we can safely
  // reset the command buffer to begin recording again.
  VK_CHECK(vkResetCommandBuffer(cmd, 0));

  //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
  VkCommandBufferBeginInfo cmd_begin_info = {};
  cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_begin_info.pNext = nullptr;
  cmd_begin_info.pInheritanceInfo = nullptr;
  cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  //start the command buffer recording
  VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

  vkutil::transition_image(cmd, swapchain_imgs[swapchain_img_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  //make a clear-color from frame number. This will flash with a 120 frame period.
  VkClearColorValue clearValue;
  float flash = std::abs(std::sin(frame_number / 120.f));
  clearValue = { { 0.0f, 0.0f, flash, 1.0f } };


  VkImageSubresourceRange clear_range {};
  clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  clear_range.baseMipLevel = 0;
  clear_range.levelCount = VK_REMAINING_MIP_LEVELS;
  clear_range.baseArrayLayer = 0;
  clear_range.layerCount = VK_REMAINING_ARRAY_LAYERS;

  vkCmdClearColorImage(cmd, swapchain_imgs[swapchain_img_index], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clear_range);
  vkutil::transition_image(cmd, swapchain_imgs[swapchain_img_index], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VK_CHECK(vkEndCommandBuffer(cmd));


  // submit stage

  VkCommandBufferSubmitInfo cmd_submit_info = {};
  cmd_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cmd_submit_info.pNext = nullptr;
  cmd_submit_info.commandBuffer = cmd;
  cmd_submit_info.deviceMask = 0;

  VkSemaphoreSubmitInfo wait_sema_info = {};
  wait_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  wait_sema_info.pNext = nullptr;
  wait_sema_info.semaphore = get_current_frame().swapchain_semaphore;
  wait_sema_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
  wait_sema_info.deviceIndex = 0;
  wait_sema_info.value = 1;

  VkSemaphoreSubmitInfo signal_sema_info = {};
  signal_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  signal_sema_info.pNext = nullptr;
  signal_sema_info.semaphore = get_current_frame().render_semaphore;
  signal_sema_info.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
  signal_sema_info.deviceIndex = 0;
  signal_sema_info.value = 1;


  VkSubmitInfo2 submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  submit_info.pNext = nullptr;

  submit_info.waitSemaphoreInfoCount = 1;
  submit_info.pWaitSemaphoreInfos = &wait_sema_info;

  submit_info.signalSemaphoreInfoCount = 1;
  submit_info.pSignalSemaphoreInfos = &signal_sema_info;

  submit_info.commandBufferInfoCount = 1;
  submit_info.pCommandBufferInfos = &cmd_submit_info;

  VK_CHECK(vkQueueSubmit2(graphics_queue, 1, &submit_info, get_current_frame().render_fence));

  // present
  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.pNext = nullptr;
  present_info.pSwapchains = &swapchain;
  present_info.swapchainCount = 1;

  present_info.pWaitSemaphores = &get_current_frame().render_semaphore;
  present_info.waitSemaphoreCount = 1;

  present_info.pImageIndices = &swapchain_img_index;

  VK_CHECK(vkQueuePresentKHR(graphics_queue, &present_info));
  frame_number++;
}

void renderer::cleanup() {
  DEBUG("cleanup");
  vkDeviceWaitIdle(device);

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    vkDestroyCommandPool(device, frames[i].command_pool, nullptr);

    vkDestroyFence(device, frames[i].render_fence, nullptr);
    vkDestroySemaphore(device, frames[i].swapchain_semaphore, nullptr);
    vkDestroySemaphore(device, frames[i].render_semaphore, nullptr);
  }

  destroy_swapchain();

  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyDevice(device, nullptr);

  vkb::destroy_debug_utils_messenger(instance, debug_messenger);
  vkDestroyInstance(instance, nullptr);
}

void renderer::destroy_swapchain() {
  vkDestroySwapchainKHR(device, swapchain, nullptr);

  for (int i = 0; i < swapchain_imgs_views.size(); i++) {
    vkDestroyImageView(device, swapchain_imgs_views[i], nullptr);
  }
}
