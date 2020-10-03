#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#include <string>

extern "C"
{
#include "skynet_server.h"
#include "skynet.h"
#include "skynet_socket.h"
#include "skynet_timer.h"
#include "skynet_handle.h"
#include "skynet_env.h"
}

using namespace std;

class Watchdog
{
public:
    Watchdog();
    bool dog_init(const string& parm);

    void watchdog_poll(const char* data, uint32_t size, uint32_t source, int session, int type);
    bool service_init(skynet_context* ctx, const void* parm, int len);
};

#endif