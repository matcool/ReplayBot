#ifndef __UTILS_HPP
#define __UTILS_HPP
#include "includes.h"

template <typename T, typename R>
T cast(R const v) { return reinterpret_cast<T>(v); }

template <typename T>
T* readPtr(uintptr_t addr) { return cast<T*>(addr); }

template <typename T>
T read(uintptr_t addr) { return *readPtr<T>(addr); }

inline uintptr_t follow(uintptr_t addr) { return read<uintptr_t>(addr); }

#endif