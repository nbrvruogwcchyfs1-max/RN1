#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>

static DWORD find_roblox() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    DWORD pid = 0;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (wcscmp(pe.szExeFile, L"RobloxPlayerBeta.exe") == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

int main() {
    DWORD pid = find_roblox();
    if (!pid) {
        std::cout << "Roblox not running\n";
        return 1;
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        std::cout << "OpenProcess failed: " << GetLastError() << "\n";
        return 1;
    }

    const char* dllPath = "C:\\AlphaExec\\alphaexec.dll";
    size_t pathSize = strlen(dllPath) + 1;

    void* remoteMem = VirtualAllocEx(hProc, nullptr, pathSize,
                                     MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMem) {
        std::cout << "VirtualAllocEx failed\n";
        return 1;
    }

    WriteProcessMemory(hProc, remoteMem, dllPath, pathSize, nullptr);

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    void* loadLib = GetProcAddress(k32, "LoadLibraryA");

    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0,
                                        (LPTHREAD_START_ROUTINE)loadLib,
                                        remoteMem, 0, nullptr);
    if (!hThread) {
        std::cout << "CreateRemoteThread failed: " << GetLastError() << "\n";
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);
    std::cout << "Injected successfully\n";

    CloseHandle(hThread);
    CloseHandle(hProc);
    return 0;
}
