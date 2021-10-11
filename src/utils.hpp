#pragma once
#include "includes.h"
#include <vector>

template <typename T, typename R>
T cast(R const v) { return reinterpret_cast<T>(v); }

template <typename T>
T* read_ptr(uintptr_t addr) { return cast<T*>(addr); }

template <typename T>
T read(uintptr_t addr) { return *read_ptr<T>(addr); }

inline uintptr_t follow(uintptr_t addr) { return read<uintptr_t>(addr); }

inline void patch(void* loc, std::vector<std::uint8_t> bytes) {
    auto size = bytes.size();
    DWORD old_prot;
    VirtualProtect(loc, size, PAGE_EXECUTE_READWRITE, &old_prot);
    memcpy(loc, bytes.data(), size);
    VirtualProtect(loc, size, old_prot, &old_prot);
}

template <typename R, typename U>
R& from_offset(U base, int offset) {
    return *cast<R*>(cast<intptr_t>(base) + offset);
}

std::string narrow(const wchar_t* str);
inline auto narrow(const std::wstring& str) { return narrow(str.c_str()); }
std::wstring widen(const char* str);
inline auto widen(const std::string& str) { return widen(str.c_str()); }
