#ifndef __UTILS_HPP
#define __UTILS_HPP
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

inline int dir_files_count(std::filesystem::path path) {
    if (std::filesystem::is_directory(path)) {
        auto iter = std::filesystem::directory_iterator(path);
        int count = 0;

        for (auto& entry : iter) {
            if (entry.is_regular_file()) {
                ++count;
            }
        }

        return count;
    }

    return 0;
}

#endif