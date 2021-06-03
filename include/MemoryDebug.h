#ifndef DEBUG_H_
#define DEBUG_H_

#include <cstdio>
#include <cstdint>
#include <cstring>
#include "spdlog/spdlog.h"

struct AllocationMetrics {
    uint32_t m_TotalAllocated = 0;
    uint32_t m_TotalFreed = 0;

    [[nodiscard]] uint32_t CurrentUsage() const { return m_TotalAllocated - m_TotalFreed; }
};

static AllocationMetrics s_AllocationMetrics;

inline void *operator new(size_t size) {
    s_AllocationMetrics.m_TotalAllocated += size;
    return malloc(size);
}

inline void operator delete(void *memory, size_t size) {
    s_AllocationMetrics.m_TotalFreed += size;

    free(memory);
}

static void PrintMemoryUsage(std::string_view prefix) {
    spdlog::info("{} Memory Usage : {}", prefix, s_AllocationMetrics.CurrentUsage());
}

template<typename T>
auto ALLOCATION_PRINT_V(T &&func) {
    try {
        auto new_function = [func = std::forward<T>(func)](auto &&... args) {
            PrintMemoryUsage("BEFORE");
            func(std::forward<decltype(args)>(args)...);
            PrintMemoryUsage("AFTER");
        };
        return new_function;
    }
    catch (const std::exception &ex) {
        std::cout << ex.what() << "\n";
    }
}

template<typename T>
auto ALLOCATION_PRINT_R(T &&func) {
    try {
        auto new_function = [func = std::forward<T>(func)](auto &&... args) {
            PrintMemoryUsage("BEFORE");
            auto result = func(std::forward<decltype(args)>(args)...);
            PrintMemoryUsage("AFTER");
            return result;

        };
        return new_function;
    }
    catch (const std::exception &ex) {
        std::cout << ex.what() << "\n";
    }
}

template<typename T>
auto ALLOCATION_CHECK_R(T &&func) {
    try {
        auto new_function = [func = std::forward<T>(func)](auto &&... args) {
            uint32_t UsageBefore = s_AllocationMetrics.CurrentUsage();
            auto result = func(std::forward<decltype(args)>(args)...);
            uint32_t UsageAfter = s_AllocationMetrics.CurrentUsage();
            uint32_t MemoryIncrease = UsageAfter - UsageBefore;

            if (MemoryIncrease > 0) {
                spdlog::critical("Before : {}", UsageBefore);
                spdlog::critical("After : {}", UsageAfter);
                spdlog::critical("Memory Increase : {}", MemoryIncrease);
            }
            return result;
        };
        return new_function;
    }
    catch (const std::exception &ex) {
        std::cout << ex.what() << "\n";
    }
}

template<typename T>
auto ALLOCATION_CHECK_V(T &&func) {
    try {
        auto new_function = [func = std::forward<T>(func)](auto &&... args) {
            uint32_t UsageBefore = s_AllocationMetrics.CurrentUsage();
            func(std::forward<decltype(args)>(args)...);
            uint32_t UsageAfter = s_AllocationMetrics.CurrentUsage();
            uint32_t MemoryIncrease = UsageAfter - UsageBefore;

            if (MemoryIncrease > 0) {
                spdlog::critical("Before : {}", UsageBefore);
                spdlog::critical("After : {}", UsageAfter);
                spdlog::critical("Memory Increase : {}", MemoryIncrease);
            }
        };
        return new_function;
    }
    catch (const std::exception &ex) {
        std::cout << ex.what() << "\n";
    }
}

#endif // DEBUG_H_