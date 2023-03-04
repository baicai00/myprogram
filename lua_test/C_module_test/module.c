/*
 * 实现供Lua调用的C模块
 */
#include "lua.h"
#include "lauxlib.h"

static int module_add(lua_State *L)
{
	double arg1 = luaL_checknumber(L, 1);
	double arg2 = luaL_checknumber(L, 2);

	double ret = arg1 + arg2;
	lua_pushnumber(L, ret);
    lua_pushstring(L, "hello world");

	return 2;
}

static int module_sub(lua_State *L)
{
	double arg1 = luaL_checknumber(L, 1);
	double arg2 = luaL_checknumber(L, 2);

	double ret = arg1 - arg2;
	lua_pushnumber(L, ret);

	return 1;
}

LUA_API int luaopen_module_core_aa(lua_State *L)
{
	luaL_Reg l[] = {
		{ "madd", module_add },
		{ "msub", module_sub },
		{ NULL, NULL},
	};

	lua_createtable(L, 0, sizeof(l)/sizeof(l[0]) - 1);
	luaL_setfuncs(L, l, 0);

	return 1;
}