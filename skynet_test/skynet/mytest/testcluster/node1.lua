require "skynet.manager"
local skynet = require "skynet"
local cluster = require "skynet.cluster"
local CMD = {}

function CMD.ping(msg)
    skynet.error("node1 recv ping msg:", msg)
    return "node1 pong" .. os.time()
end


skynet.start(function ()
    skynet.dispatch("lua", function (session, source, cmd, ...)
        local f = CMD[cmd]
        if f then
            local ret = f(...)
            if source ~= 0 then
                skynet.ret(skynet.pack(ret))
            end
        else
            error(string.format("Unknown command %s", tostring(cmd)))
        end
    end)

    -- skynet.register(".testnode1")

    skynet.newservice("debug_console", 10000)

    cluster.reload({
        node1 = "127.0.0.1:2528",
        node2 = "127.0.0.1:2529",
    })

    cluster.register("testnode1")

    cluster.open("node1")
end)