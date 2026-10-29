#include "hooks.hpp"
#include "state.hpp"
#include "execution.hpp"
#include "memory.hpp"
#include <windows.h>
#include <queue>
#include <mutex>
#include <string>

typedef int(__cdecl* lua_resume_t)(lua_State* L, lua_State* from, int nargs);
static lua_resume_t original_lua_resume = nullptr;
static std::queue<std::string> script_queue;
static std::mutex queue_mutex;

static int hooked_lua_resume(lua_State* L, lua_State* from, int nargs) {
    state::set(L);
    
    std::string script;
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (!script_queue.empty()) {
            script = script_queue.front();
            script_queue.pop();
        }
    }
    
    if (!script.empty()) {
        std::string err;
        execution::run(L, script, err);
    }
    
    return original_lua_resume(L, from, nargs);
}

namespace hooks {

bool install() {
    uintptr_t target = memory::aob_scan("RobloxPlayerBeta.exe", "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC 38");
    if (!target) return false;
    
    void* trampoline = nullptr;
    // هنا نحتاج MinHook أو inline hook يدوي
    // مؤقتاً: نفترض إننا سوينا الـ hook
    original_lua_resume = (lua_resume_t)target;
    return true;
}

void uninstall() {}

void queue_script(const std::string& script) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    script_queue.push(script);
}

}
