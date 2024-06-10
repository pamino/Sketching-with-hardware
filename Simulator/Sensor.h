#pragma once

#include <iostream>

enum SensorDirection {
  TOPRIGHT,
  TOPLEFT,
  BOTTOMRIGHT,
  BOTTOMLEFT,
  TOP,
};

#ifdef _WIN32
#include <intrin.h>
#define DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#endif

#define my_assert(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << "Assertion failed: " << #condition << ", file " << __FILE__ << ", line " << __LINE__ << std::endl; \
            DEBUG_BREAK(); \
        } \
    } while (0)