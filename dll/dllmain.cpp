#include <windows.h>
#include "hooks.hpp"
#include "ui.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        CreateThread(nullptr, 0, [](LPVOID) -> DWORD {
            Sleep(3000);

            if (!hooks::install()) {
                return 1;
            }

            ui::start();
            return 0;
        }, nullptr, 0, nullptr);
    }
    return TRUE;
}
