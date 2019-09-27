#include <stdio.h>
#include "lua.h"

int main()
{
    lua_State *L = luaL_newstate();
    
    luaL_openlibas(L);

    luaL_dofile(L, "module.lua");

    lua_getglobal(L, "Obj1");
    if (lua_isinteger(L, -1) == 1) {
        printf("Obj1=%d\n", lua_tointeger(L, -1));
    }

    lua_close(L);

    return 0;
}