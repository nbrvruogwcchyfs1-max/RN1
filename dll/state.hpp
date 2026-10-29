#pragma once
#include <lua.h>

namespace state {
    void set(lua_State* L);
    lua_State* get();
}
