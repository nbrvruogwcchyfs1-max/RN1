#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace memory {
    uintptr_t aob_scan(const std::string& module_name, const std::string& pattern);
    uintptr_t get_module_base(const std::string& module_name);

    template<typename T>
    T read(uintptr_t address);

    template<typename T>
    bool write(uintptr_t address, T value);
}
