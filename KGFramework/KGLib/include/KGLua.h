#pragma once
extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}
#pragma comment(lib, "lua54.lib")

namespace KG::Lua
{
    inline lua_State* OpenCommonState()
    {
        lua_State* state = luaL_newstate();
        luaL_openlibs(state);
        return state;
    }
};