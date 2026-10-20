#pragma once

#include <iostream>

// CHECK stays active in Debug AND Release builds (unlike standard assert).
#define CHECK(condition) \
    do \
    { \
        if (!(condition)) \
        { \
            std::cerr << "CHECK failed: " << #condition << " at " \
                      << __FILE__ << ":" << __LINE__ << "\n"; \
            return 1; \
        } \
    } while (false)
