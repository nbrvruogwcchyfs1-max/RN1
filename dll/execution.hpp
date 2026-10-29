#pragma once
#include <string>
#include <lua.h>
#include <luacode.h>

namespace execution {
    bool run(lua_State* L, const std::string& source, std::string& error);
    bool execute(const std::string& source);
}
