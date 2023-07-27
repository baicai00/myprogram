require "skynet.manager"
local skynet = require "skynet"
local cluster = require "skynet.cluster"

local function cancelable_timeout(ti, func, ...)
    local args = table.pack(...)
    local function cb()
      if func then
        func(table.unpack(args))
      end
    end
    local function cancel()
      func = nil
    end
    skynet.timeout(ti, cb)
    return cancel
end

local function repeat_ping(proxy)
    --local status, ret = pcall(skynet.call, proxy, "lua", "ping", "my name is node2")
    --local ret = cluster.call("node1", "testnode1", "ping", "my name is node2")
    local status, ret = pcall(cluster.call, "node1", "@testnode1", "ping", "my name is node2" .. os.time())
    if status then
        skynet.error(ret)
    else
        skynet.error("pcall failed. errmsg:", ret)
    end

    cancelable_timeout(500, repeat_ping, proxy)
end

skynet.start(function ()
    cluster.reload({
        node1 = "127.0.0.1:2528",
        node2 = "127.0.0.1:2529",
    })

    cluster.open("node2")
    local proxy = cluster.proxy("node1@testnode1")

    cancelable_timeout(100, repeat_ping, proxy)
end)