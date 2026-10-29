#include "execution.hpp"
#include "state.hpp"
#include <cstdlib>
#include <cstring>

namespace execution {

bool run(lua_State* L, const std::string& source, std::string& error) {
    error.clear();
    if (!L) { error = "no Luau state"; return false; }

    lua_State* thread = lua_newthread(L);
    if (!thread) { error = "lua_newthread failed"; return false; }

    lua_CompileOptions options{};
    options.optimizationLevel = 1;
    options.debugLevel = 1;

    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(source.c_str(), source.size(), &options, &bytecodeSize);
    
    if (!bytecode) {
        error = "compiler failed";
        return false;
    }

    int loaded = luau_load(thread, "=AlphaExec", bytecode, bytecodeSize, 0);
    std::free(bytecode);

    if (loaded != 0) {
        const char* msg = lua_tostring(thread, -1);
        error = msg ? msg : "luau_load failed";
        lua_pop(thread, 1);
        return false;
    }

    int status = lua_pcall(thread, 0, 0, 0);

    if (status == LUA_YIELD) {
        error = "script yielded (task.wait unsupported)";
        return false;
    }
    if (status != 0) {
        const char* msg = lua_tostring(thread, -1);
        error = msg ? msg : "runtime error";
        lua_pop(thread, 1);
        return false;
    }

    return true;
}

bool execute(const std::string& source) {
    lua_State* L = state::get();
    if (!L) return false;
    std::string err;
    return run(L, source, err);
}

}
