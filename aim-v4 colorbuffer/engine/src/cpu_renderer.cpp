#include "cpu_renderer.h"
//#define VMA_IMPLEMENTATION
//#include <vma/vk_mem_alloc.h>

cpu_renderer* cpu_rend = nullptr;

void init_cpu_renderer(int width, int height) {
  static cpu_renderer local_renderer {.pixels = std::vector<uint32_t>(width * height), .color = 0, .config = {.width = width, .height = height}};
  cpu_rend = &local_renderer;
  //cpu_rend = new cpu_renderer {.pixels = std::vector<uint32_t>(1700 * 900, 0), .color = 0};
}


void cpu_clear(int width, int height, int color) {
  if (width != cpu_rend->config.width || height != cpu_rend->config.height){
    cpu_rend->pixels = std::vector<uint32_t>(width * height, color);
    cpu_rend->config.width = width;
    cpu_rend->config.height = height;
  }else{
    std::fill(cpu_rend->pixels.begin(), cpu_rend->pixels.end(), color);
  }

}

void cpu_draw_rect(int x, int y, int w, int h, uint32_t color) {
  INFO("inside cpu_draw_rect, x: %d, y: %d, w: %d, h: %d", x, y, w, h);
  if (color == 0) {
    color = cpu_rend->color;
  }

  int texWidth = 1700;
  int res1 = x + w;
  int res2 = y + h;
  for (int i = y; i < res2; i++) {
      for (int j = x; j < res1; j++) {
  	  int index = (i * texWidth + j);
  	  cpu_rend->pixels[index] = color;
  	}
  }
}


void cpu_set_draw_color(uint32_t new_color) {
  cpu_rend->color = new_color;
}


void cpu_draw_point(int x, int y, uint32_t color) {
    if (color == 0) {
      color = cpu_rend->color;
    };
    //int texWidth = draw_image.imageExtent.width;
    int texWidth = 1700;
    cpu_rend->pixels[y * texWidth + x] = color;
}



//void cpu_submit(VkCommandBuffer cmd, AllocatedImage* draw_image, VmaAllocator* allocator) {
//  VkDeviceSize image_size = draw_image->imageExtent.height * draw_image->imageExtent.width * 4;
//
//    // Create the staging buffer
//    VkBuffer stagingBuffer;
//    VmaAllocation stagingBufferAllocation;
//    VmaAllocationCreateInfo stagingAllocInfo = {};
//    stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
//
//    VkBufferCreateInfo bufferInfo = {};
//    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//    bufferInfo.size = image_size;
//    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//
//    vmaCreateBuffer(*allocator, &bufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingBufferAllocation, nullptr);
//
//    // Map the buffer and copy data
//    void* data;
//    vmaMapMemory(*allocator, stagingBufferAllocation, &data);
//    //memcpy(data, cpu_renderer.pixels.data(), static_cast<size_t>(image_size));
//    memcpy(data, cpu_rend->pixels.data(), static_cast<size_t>(image_size));
//    vmaUnmapMemory(*allocator, stagingBufferAllocation);
//
//
//    // this is like a begin and end commands in vulkan-tutorial
//    immediate_submit([&](VkCommandBuffer cmd) {
//    	 // Transition image layout and copy buffer to image
//    	vkutil::transition_image(cmd, draw_image->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//
//    	VkBufferImageCopy region{ 0 };
//    	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    	region.imageSubresource.mipLevel = 0;
//    	region.imageSubresource.baseArrayLayer = 0;
//    	region.imageSubresource.layerCount = 1;
//    	region.imageOffset = { 0, 0, 0 };
//    	region.imageExtent = {
//    	    draw_image->imageExtent.width,
//    	    draw_image->imageExtent.height,
//    	    1
//    	};
//
//    	vkCmdCopyBufferToImage(cmd, stagingBuffer, draw_image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
//    	vkutil::transition_image(cmd, draw_image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
//	// maybe just copy it into the swapchain? and not use a draw image
//    });
//    // this is like a begin and end commands in vulkan-tutorial
//
//    // Cleanup
//    vmaDestroyBuffer(*allocator, stagingBuffer, stagingBufferAllocation);
//}
