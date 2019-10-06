#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

// 在该函数中测试C语言调用lua
void CcallLua()
{
    lua_State *L = luaL_newstate();
    
    luaL_openlibs(L);

    luaL_dofile(L, "module.lua");

    ////////////////////////////////////////////////////
    // 读取全局变量Obj1的值
    lua_getglobal(L, "Obj1");
    if (lua_isnumber(L, -1) == 1) {
        printf("Obj1=%d\n", (int)lua_tointeger(L, -1));
    }

    ////////////////////////////////////////////////////
    // 调用全局函数Add
    lua_settop(L, 0);
    lua_getglobal(L, "Add");
    lua_pushinteger(L, 5);
    lua_pushinteger(L, 10);
    lua_pcall(L, 2, 1, 0);
    if (lua_isnumber(L, -1) == 1) {
        printf("Add ret=%d\n", (int)lua_tointeger(L, -1));
    }

    /////////////////////////////////////////////////////
    // 调用表中的函数(这些调用的关健是需要弄清楚表tt在栈中的索引，此处tt在栈中的索引为1)
    lua_settop(L, 0);
    lua_getglobal(L, "tt");
    lua_getfield(L, 1, "show");//该函数的参数二指名表在栈中的索引，参数三指定表成员的名称，该函数将获取到的值压入栈顶
    lua_pushinteger(L, 99);
    lua_pcall(L, 1, 0, 0);//调用tt.show函数，参数二指示tt.show的参数个数，参数三指示tt.show的返回值个数，参数三是错误处理函数的索引(0表示没有)

    // 获取tt.name的值
    // 方法一
    lua_getfield(L, 1, "name");
    if (lua_isstring(L, -1) == 1) {
        printf("tt.name=%s\n", lua_tostring(L, -1));
    }
    // 方法二
    lua_pushstring(L, "name");
    lua_gettable(L, 1);//该函数的参数二指示表在栈中的索引,该函数会读取栈顶元素的值，并将该值做为表索引读取表中的值，最后把读到的值写入栈顶。假如栈顶元素的值为"name",那么将读取tt["name"]的值，并该值压入栈顶
    if (lua_isstring(L, -1) == 1) {
        printf("tt[name]=%s\n", lua_tostring(L, -1));
    }

    // 获取tt[1]的值
    lua_pushinteger(L, 1);
    lua_gettable(L, 1);
    if (lua_isnumber(L, -1) == 1) {
        printf("tt[1]=%d\n", (int)lua_tointeger(L, -1));
    }

    lua_close(L);
}

//////////////////////////////////////////////////////////////////////////////////////
// 在lua5.1中规定可以被Lua调用的C函数的形式：typedef int (*lua_CFunction) (lua_State *L);
int cfunc_add(lua_State *L)
{
    int n = lua_gettop(L); // 获取参数个数
    double sum = 0;
    int i;
    for (i = 1; i <= n; ++i) {
        sum += lua_tonumber(L, i);
    }
    lua_pushnumber(L, sum);//将返回值压入栈中
    return 1;// 返回返回值的个数
}
// 在该函数中实现lua调用C
void LuaCallC()
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    // 将C函数设置到Lua的全局变量cfunc_add_v
    lua_register(L, "cfunc_add_v", cfunc_add);

    luaL_dofile(L, "callC.lua");
    
    lua_getglobal(L, "aa");
    if (lua_isnumber(L, -1) == 1) {
        printf("aa=%f\n", lua_tonumber(L, -1));
    }

    lua_close(L);
}

int main()
{
    LuaCallC();
    return 0;
}