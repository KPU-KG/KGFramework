#include "hash.h"
#include "KGLua.h"
#include <iostream>

int main()
{
    lua_State* luaState = KG::Lua::OpenCommonState();
    int sx = 1600;
    int sy = 900;
    std::string command = "return sx + sy";
    lua_pushinteger(luaState, sx);
    lua_setglobal(luaState, "sx");
    lua_pushinteger(luaState, sy);
    lua_setglobal(luaState, "sy");
    luaL_loadstring(luaState, command.c_str());
    lua_call(luaState, 0, 1);
    int result = lua_tonumber(luaState, -1);
    lua_settop(luaState, 0);
    std::cout << result << std::endl;
    system("pause");
}