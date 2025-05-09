#pragma once
#include<iostream>

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
