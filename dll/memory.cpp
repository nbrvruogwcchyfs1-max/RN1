#include "memory.hpp"
#include <windows.h>
#include <psapi.h>
#include <sstream>

namespace memory {

uintptr_t get_module_base(const std::string& module_name) {
    HMODULE modules[1024];
    DWORD needed = 0;
    if (!EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed))
        return 0;
    DWORD count = needed / sizeof(HMODULE);
    for (DWORD i = 0; i < count; i++) {
        char name[MAX_PATH] = {0};
        if (GetModuleBaseNameA(GetCurrentProcess(), modules[i], name, MAX_PATH)) {
            if (_stricmp(name, module_name.c_str()) == 0) {
                return reinterpret_cast<uintptr_t>(modules[i]);
            }
        }
    }
    return 0;
}

static bool parse_pattern(const std::string& pattern, std::vector<uint8_t>& bytes, std::vector<bool>& mask) {
    std::istringstream iss(pattern);
    std::string token;
    while (iss >> token) {
        if (token == "?" || token == "??") {
            bytes.push_back(0);
            mask.push_back(false);
        } else {
            bytes.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, 16)));
            mask.push_back(true);
        }
    }
    return !bytes.empty();
}

uintptr_t aob_scan(const std::string& module_name, const std::string& pattern) {
    uintptr_t base = get_module_base(module_name);
    if (!base) return 0;
    
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;
    
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;
    
    size_t module_size = nt->OptionalHeader.SizeOfImage;
    std::vector<uint8_t> bytes;
    std::vector<bool> mask;
    if (!parse_pattern(pattern, bytes, mask)) return 0;
    
    const uint8_t* data = reinterpret_cast<const uint8_t*>(base);
    size_t pattern_size = bytes.size();
    if (pattern_size == 0 || pattern_size > module_size) return 0;
    
    for (size_t i = 0; i <= module_size - pattern_size; i++) {
        bool found = true;
        for (size_t j = 0; j < pattern_size; j++) {
            if (mask[j] && data[i + j] != bytes[j]) {
                found = false;
                break;
            }
        }
        if (found) return base + i;
    }
    return 0;
}

template<typename T>
T read(uintptr_t address) {
    if (!address) return T{};
    __try {
        return *reinterpret_cast<T*>(address);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return T{};
    }
}

template<typename T>
bool write(uintptr_t address, T value) {
    if (!address) return false;
    DWORD old_protect = 0;
    if (!VirtualProtect(reinterpret_cast<void*>(address), sizeof(T), PAGE_EXECUTE_READWRITE, &old_protect))
        return false;
    *reinterpret_cast<T*>(address) = value;
    VirtualProtect(reinterpret_cast<void*>(address), sizeof(T), old_protect, &old_protect);
    return true;
}

template int read<int>(uintptr_t);
template float read<float>(uintptr_t);
template double read<double>(uintptr_t);
template uintptr_t read<uintptr_t>(uintptr_t);
template uint8_t read<uint8_t>(uintptr_t);

template bool write<int>(uintptr_t, int);
template bool write<float>(uintptr_t, float);
template bool write<uintptr_t>(uintptr_t, uintptr_t);
template bool write<uint8_t>(uintptr_t, uint8_t);

}
