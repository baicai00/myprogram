require 'skynet.manager'
local skynet = require "skynet"

local gtables = {}

skynet.register_protocol {
    name = "text",
    id = skynet.PTYPE_TEXT,
    pack = function(...) return ... end,
    unpack = skynet.tostring,
}

function gtables.skynet_start()
    skynet.register(".main")
    skynet.newservice("debug_console", tonumber(skynet.getenv("debug_port")))

    skynet.dispatch("lua", function(session, address, cmd, ...)
        local f = CMD[cmd]
        if f then
            local ret = f(...)
            if ret ~= false then
                skynet.ret(skynet.pack(ret))
            end
        else
            error(string.format("Unknown command %s", tostring(cmd)))
        end
    end)
end

skynet.start(gtables.skynet_start)