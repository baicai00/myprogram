#include <stdio.h>
#include <string.h>
#include "Watchdog.h"

extern "C"
{
#include "skynet.h"
#include "skynet_socket.h"
#include "skynet_server.h"
#include "skynet_handle.h"
#include "skynet_env.h"
#include "skynet_timer.h"
}

using namespace std;

extern "C"
{
    static int watchdog_cb(struct skynet_context * ctx, void * ud, int type, int session, uint32_t source, const void * msg, size_t sz)
    {
        Watchdog * dog = (Watchdog*)ud;
        dog->watchdog_poll((const char*)msg, sz, source, session, type);
        return 0;
    }

    Watchdog* watchdog_create()
    {
        Watchdog * dog = new Watchdog();
        return dog;
    }

    void watchdog_release(Watchdog* dog)
    {
        delete dog;
    }

    int watchdog_init(Watchdog* dog, struct skynet_context* ctx, char* parm)
    {
        unsigned handle = skynet_context_handle(ctx);
        char sendline[100];
        snprintf(sendline, sizeof(sendline), "%s %u", parm, handle);
        printf("watchdog_init sendline=%s\n", sendline);
        // watchdog新建一个gate服务
        struct skynet_context* gate_ctx = skynet_context_new("gate", sendline);
        if (gate_ctx == NULL)
        {
            printf("create gate error\n");
            return -1;
        }
        uint32_t gate = skynet_context_handle(gate_ctx);
        char temp[100];
        sprintf(temp, "%u", gate);
        if (!dog->service_init(ctx, temp, strlen(temp)))
        {
            printf("service init failed.\n");
            return -1;
        }

        skynet_callback(ctx, dog, watchdog_cb);
        return 0;
    }
}