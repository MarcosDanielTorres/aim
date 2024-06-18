#include "renderer.h"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>


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
  init_descriptors();
  init_pipelines();
  init_imgui(window);
  //init_default_data();
}

void renderer::init_imgui(SDL_Window* window) {
  // 1: create descriptor pool for IMGUI
  //  the size of the pool is very oversize, but it's copied from imgui demo
  //  itself.
  VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
  	{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
  	{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
  	{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
  	{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
  	{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
  	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
  	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
  	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
  	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
  	{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000;
  pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;

  VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool));

  // 2: initialize imgui library

  // this initializes the core structures of imgui
  ImGui::CreateContext();

  // this initializes imgui for SDL
  ImGui_ImplSDL2_InitForVulkan(window);

  // this initializes imgui for Vulkan
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = instance;
  init_info.PhysicalDevice = physical_device;
  init_info.Device = device;
  init_info.Queue = graphics_queue;
  init_info.DescriptorPool = imguiPool;
  init_info.MinImageCount = 3;
  init_info.ImageCount = 3;
  init_info.UseDynamicRendering = true;

  //dynamic rendering parameters for imgui to use
  init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
  init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
  init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchain_img_fmt;


  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  ImGui_ImplVulkan_Init(&init_info);

  ImGui_ImplVulkan_CreateFontsTexture();

  }


void renderer::init_pipelines()
{
  init_background_pipelines();
}

void renderer::init_background_pipelines()
{
  	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &_drawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;

	VK_CHECK(vkCreatePipelineLayout(device, &computeLayout, nullptr, &_gradientPipelineLayout));
	//layout code
	VkShaderModule computeDrawShader;
	// i should have a way of setting this root folder for assets
	if (!vkutil::load_shader_module("engine/assets/shaders/gradient.comp.spv", device, &computeDrawShader))
	{
		FATAL("Error when building the compute shader");
	}

	VkPipelineShaderStageCreateInfo stageinfo{};
	stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageinfo.pNext = nullptr;
	stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageinfo.module = computeDrawShader;
	stageinfo.pName = "main";

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = _gradientPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;
	
	VK_CHECK(vkCreateComputePipelines(device,VK_NULL_HANDLE,1,&computePipelineCreateInfo, nullptr, &_gradientPipeline));


	vkDestroyShaderModule(device, computeDrawShader, nullptr);

	main_deletion_queue.push_pipelines_layouts(_gradientPipelineLayout);
	main_deletion_queue.push_pipelines(_gradientPipeline);
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

  // initialize the memory allocator
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = physical_device;
  allocatorInfo.device = device;
  allocatorInfo.instance = instance;
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  vmaCreateAllocator(&allocatorInfo, &allocator);

  main_deletion_queue.device = device;
  main_deletion_queue.allocator = allocator;
  for (int i = 0; i < FRAME_OVERLAP; i++) {
    frames[i].frame_deletion_queue.device = device;
    frames[i].frame_deletion_queue.allocator = allocator;
  }
  //main_deletion_queue.push_function([&]() {
  //    vmaDestroyAllocator(_allocator);
  //});
  main_deletion_queue.push_allocator(allocator);
  
  INFO("Init vulkan!");
}

void renderer::draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView){
	VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = vkinit::rendering_info(swapchain_extent, &colorAttachment, nullptr);

	vkCmdBeginRendering(cmd, &renderInfo);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	vkCmdEndRendering(cmd);
}


void renderer::init_swapchain(){
  create_swapchain(window_extent.width, window_extent.height);
  VkExtent3D drawImageExtent = {
    window_extent.width,
    window_extent.height,
    1,
  };
  //hardcoding the draw format to 32 bit float
  draw_image.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
  draw_image.imageExtent = drawImageExtent;

  VkImageUsageFlags drawImageUsages{};
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  VkImageCreateInfo rimg_info = vkinit::image_create_info(draw_image.imageFormat, drawImageUsages, drawImageExtent);

  //for the draw image, we want to allocate it from gpu local memory
  VmaAllocationCreateInfo rimg_allocinfo = {};
  rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  //allocate and create the image
  vmaCreateImage(allocator, &rimg_info, &rimg_allocinfo, &draw_image.image, &draw_image.allocation, nullptr);

  //build a image-view for the draw image to use for rendering
  VkImageViewCreateInfo rview_info = vkinit::imageview_create_info(draw_image.imageFormat, draw_image.image, VK_IMAGE_ASPECT_COLOR_BIT);

  VK_CHECK(vkCreateImageView(device, &rview_info, nullptr, &draw_image.imageView));

  //add to deletion queues
  main_deletion_queue.push_imgs_views(draw_image.imageView);
  main_deletion_queue.push_imgs(draw_image.image, draw_image.allocation);

  //vkDestroyImageView(_device, _drawImage.imageView, nullptr);
  // vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
  //});
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

  VK_CHECK(vkCreateCommandPool(device, &command_pool_info, nullptr, &_immCommandPool));

  // allocate the command buffer for immediate submits
    VkCommandBufferAllocateInfo cmd_alloc_info = {};
    cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc_info.pNext = nullptr;
    cmd_alloc_info.commandPool = _immCommandPool;
    cmd_alloc_info.commandBufferCount = 1;
    cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  VK_CHECK(vkAllocateCommandBuffers(device, &cmd_alloc_info, &_immCommandBuffer));
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

  VK_CHECK(vkCreateFence(device, &fence_create_info, nullptr, &_immFence));
  //main_deletion_queue.push_fences(_immFence);
  
}

void renderer::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) {
    VK_CHECK(vkResetFences(device, 1, &_immFence));
    VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));

    VkCommandBuffer cmd = _immCommandBuffer;

    VkCommandBufferBeginInfo cmd_begin_info = {};
    cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_begin_info.pNext = nullptr;
    cmd_begin_info.pInheritanceInfo = nullptr;
    cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    //VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);
    //VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, nullptr, nullptr);

    VkCommandBufferSubmitInfo cmd_submit_info = {};
    cmd_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cmd_submit_info.pNext = nullptr;
    cmd_submit_info.commandBuffer = cmd;
    cmd_submit_info.deviceMask = 0;

    VkSubmitInfo2 submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info.pNext = nullptr;

    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cmd_submit_info;

    // submit command buffer to the queue and execute it.
    //  _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(graphics_queue, 1, &submit_info, _immFence));

    VK_CHECK(vkWaitForFences(device, 1, &_immFence, true, 9999999999));
}

void renderer:: init_descriptors() {
  	// create a descriptor pool holding 10 sets with 1 image each
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
	};

	globalDescriptorAllocator.init_pool(device, 10, sizes);


	// make the descriptor set layout for our compute draw
	{
		DescriptorLayoutBuilder builder;
		// must match the pool. Which is why init_pool type and add_binding type are the same
		builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		_drawImageDescriptorLayout = builder.build(device, VK_SHADER_STAGE_COMPUTE_BIT, nullptr, 0);
	}
	_drawImageDescriptors = globalDescriptorAllocator.allocate(device, _drawImageDescriptorLayout);
	
	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = draw_image.imageView;
	
	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;
	
	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = _drawImageDescriptors;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(device, 1, &drawImageWrite, 0, nullptr);
	main_deletion_queue.push_descriptors_allocators(globalDescriptorAllocator);
	main_deletion_queue.push_descriptor_sets_layouts(_drawImageDescriptorLayout);
	//_mainDeletionQueue.push_function([=]()
	//{
	//    globalDescriptorAllocator.destroy_pool(_device);
	//    vkDestroyDescriptorSetLayout(_device, _drawImageDescriptorLayout, nullptr);
	//});
	
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
  get_current_frame().frame_deletion_queue.flush();

  uint32_t swapchain_img_index;
  //VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().swapchain_semaphore, nullptr, &swapchain_img_index));
  VkResult e = vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().swapchain_semaphore, nullptr, &swapchain_img_index);
  if (e == VK_ERROR_OUT_OF_DATE_KHR) {
    resize_requested = true; 
    return ;
  }

  VK_CHECK(vkResetFences(device, 1, &get_current_frame().render_fence));

  // now that we are sure that the commands finished executing, we can safely
  // reset the command buffer to begin recording again.
  VK_CHECK(vkResetCommandBuffer(get_current_frame().main_command_buffer, 0));

  //naming it cmd for shorter writing
  VkCommandBuffer cmd = get_current_frame().main_command_buffer;

  //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
  VkCommandBufferBeginInfo cmd_begin_info = {};
  cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_begin_info.pNext = nullptr;
  cmd_begin_info.pInheritanceInfo = nullptr;
  cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


draw_extent.height = std::min(swapchain_extent.height, draw_image.imageExtent.height) * renderScale;
draw_extent.width= std::min(swapchain_extent.width, draw_image.imageExtent.width) * renderScale;

  //start the command buffer recording
  VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

  vkutil::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  draw_background(cmd);

  //transition the draw image and the swapchain image into their correct transfer layouts
  vkutil::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  vkutil::transition_image(cmd, swapchain_imgs[swapchain_img_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // execute a copy from the draw image into the swapchain
  vkutil::copy_image_to_image(cmd, draw_image.image, swapchain_imgs[swapchain_img_index], draw_extent, swapchain_extent);

  // set swapchain image layout to Attachment Optimal so we can draw it
  vkutil::transition_image(cmd, swapchain_imgs[swapchain_img_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  //draw imgui into the swapchain image
  draw_imgui(cmd,  swapchain_imgs_views[swapchain_img_index]);

  // Set swapchain image layout to PRESENT so we can show it on the screen.
  vkutil::transition_image(cmd, swapchain_imgs[swapchain_img_index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);


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

  //VK_CHECK(vkQueuePresentKHR(graphics_queue, &present_info));
  VkResult presentResult = vkQueuePresentKHR(graphics_queue, &present_info);
  if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
    resize_requested = true;
  }

  frame_number++;
}

void renderer::resize_swapchain(SDL_Window* window)
{
	vkDeviceWaitIdle(device);

	destroy_swapchain();

	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	window_extent.width = w;
	window_extent.height = h;

	create_swapchain(window_extent.width, window_extent.height);
  VkExtent3D drawImageExtent = {
    window_extent.width,
    window_extent.height,
    1,
  };
  //hardcoding the draw format to 32 bit float
  draw_image.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
  draw_image.imageExtent = drawImageExtent;

	resize_requested = false;
}

void renderer::draw_background(VkCommandBuffer cmd) {
  //make a clear-color from frame number. This will flash with a 120 frame period.
  //  VkClearColorValue clearValue;
  //  float flash = std::abs(std::sin(frame_number / 120.f));
  //  clearValue = { { 0.0f, 0.0f, flash, 1.0f } };
  //
  //  VkImageSubresourceRange clear_range {};
  //  clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  //  clear_range.baseMipLevel = 0;
  //  clear_range.levelCount = VK_REMAINING_MIP_LEVELS;
  //  clear_range.baseArrayLayer = 0;
  //  clear_range.layerCount = VK_REMAINING_ARRAY_LAYERS;
  //
  //  // the clear is now done in draw_image.image instead of in the swapchain
  //  vkCmdClearColorImage(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clear_range);


  // bind the gradient drawing compute pipeline
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipeline);
 
  // bind the descriptor set containing the draw image for the compute pipeline
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipelineLayout, 0, 1, &_drawImageDescriptors, 0, nullptr);
 
  // execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
 vkCmdDispatch(cmd, std::ceil(draw_extent.width / 16.0), std::ceil(draw_extent.height / 16.0), 1);
}

void renderer::cleanup() {
  DEBUG("cleanup");
  vkDeviceWaitIdle(device);

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    vkDestroyCommandPool(device, frames[i].command_pool, nullptr);

    vkDestroyFence(device, frames[i].render_fence, nullptr);
    vkDestroySemaphore(device, frames[i].swapchain_semaphore, nullptr);
    vkDestroySemaphore(device, frames[i].render_semaphore, nullptr);

    frames[i].frame_deletion_queue.flush();
  }

  // imgui cleanup
  vkDestroyCommandPool(device, _immCommandPool, nullptr);

  vkDestroyFence(device, _immFence, nullptr);

  ImGui_ImplVulkan_Shutdown();
  vkDestroyDescriptorPool(device, imguiPool, nullptr);

  // imgui cleanup


  main_deletion_queue.flush();


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
