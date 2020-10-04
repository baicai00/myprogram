#include "Watchdog.h"
#include <string.h>

Watchdog::Watchdog()
{

}

bool Watchdog::dog_init(const string& parm)
{
    return true;
}

void Watchdog::watchdog_poll(const char* data, uint32_t size, uint32_t source, int session, int type)
{
    switch (type)
    {
    case PTYPE_TEXT:
    case PTYPE_CLIENT:
    {
        char* c = (char*)data;
        int fd = atoi(strsep(&c, " "));
        //LOG(INFO) << "PTYPE_CLIENT fd = " << fd;
        printf("watchdog_poll fd=%d\n", fd);
        break;
    }
    case PTYPE_RESPONSE:
    default:
        break;
    }
}

bool Watchdog::service_init(skynet_context* ctx, const void* parm, int len)
{
    return true;
}