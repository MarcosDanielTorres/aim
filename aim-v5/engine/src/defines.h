#pragma once
#include<iostream>

#ifdef AIM_EXPORTS
#define AIM_API __declspec(dllexport)
#else
#define AIM_API __declspec(dllimport)
#endif


AIM_API void print_322();
