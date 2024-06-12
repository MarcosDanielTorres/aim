#pragma once
#include<iostream>
#include <vulkan/vk_enum_string_helper.h>

#define VK_CHECK(exp)                            \
  do{                                            \
    VkResult result = exp;                       \
    if (result != VK_SUCCESS) {                  \
      FATAL("%s", string_VkResult(result));	 \
      abort();	                                 \
    }                                            \
  } while(0)                                     


#ifdef AIM_EXPORTS
#define AIM_API __declspec(dllexport)
#else
#define AIM_API __declspec(dllimport)
#endif
