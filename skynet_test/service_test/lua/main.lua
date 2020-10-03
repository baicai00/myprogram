local skynet = require "skynet.manager"

skynet.start(function()
    skynet.register_protocol {
        name = "text",
        id = skynet.PTYPE_TEXT,
        pack = function(...) return ... end,
        unpack = skynet.tostring,
    }

    local watchdog = assert(skynet.launch("watchdog", "0.0.0.0:".. 8888))

    skynet.exit()
end)