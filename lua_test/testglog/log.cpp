extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}
#include <glog/logging.h>

#define LOG_MESSAGE_SIZE 256
#define skynet_malloc malloc
#define skynet_free free

static int linit(lua_State *L) {
    const char* dir = "./log";
    const char* name = "test";
    int logbufsecs = 1;
    FLAGS_log_dir = dir;
    FLAGS_logbufsecs = logbufsecs;
    FLAGS_max_log_size = 10; // 单位M
    google::InitGoogleLogging(name);
    return 0;
}

static char * skynet_strdup(const char *str) {
    size_t sz = strlen(str);
    char * ret = (char*)skynet_malloc(sz+1);
    memcpy(ret, str, sz+1);
    return ret;
}

static bool format_message(char*& data, const char *msg, ...) {
    char tmp[LOG_MESSAGE_SIZE];

    va_list ap;

    va_start(ap,msg);
    int len = vsnprintf(tmp, LOG_MESSAGE_SIZE, msg, ap);
    va_end(ap);
    if (len >=0 && len < LOG_MESSAGE_SIZE) {
        data = skynet_strdup(tmp);
    } else {
        int max_size = LOG_MESSAGE_SIZE;
        for (;;) {
            max_size *= 2;
            data = (char*)skynet_malloc(max_size);
            va_start(ap,msg);
            len = vsnprintf(data, max_size, msg, ap);
            va_end(ap);
            if (len < max_size) {
                break;
            }
            skynet_free(data);
        }
    }
    if (len < 0) {
        skynet_free(data);
        perror("vsnprintf error :");
        return false;
    }
    return true;
}

static int lerror(lua_State *L) {
    int n = lua_gettop(L);
    if (n < 1) {
        return 0;
    }

    luaL_Buffer b;
    luaL_buffinit(L, &b);
    int i;
    for (i=1; i<=n; i++) {
        luaL_tolstring(L, i, NULL);
        luaL_addvalue(&b);
        if (i<n) {
            luaL_addchar(&b, ' ');
        }
    }
    luaL_pushresult(&b);
    char *data = NULL;
    bool ret = format_message(data, "%s", lua_tostring(L, -1));
    if (ret)
    {
        LOG(ERROR) << data;
        skynet_free(data);
    }
    //skynet_error(context, "%s", lua_tostring(L, -1));
    return 0;
}

extern "C" int luaopen_beelog(lua_State *L) {
    luaL_checkversion(L);

    luaL_Reg l[] = {
        {"init", linit},
        //{"info", linfo},
        //{"warning", lwarning},
        {"error", lerror},
        { NULL, NULL },
    };

    lua_createtable(L, 0, sizeof(l)/sizeof(l[0]) - 1);
    luaL_setfuncs(L,l,0);

    return 1;
}