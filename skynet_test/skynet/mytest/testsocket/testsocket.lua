require "skynet.manager"
local skynet = require "skynet"
local socket = require "skynet.socket"

local handler = {}

function handler.accept(id, ip)
    skynet.error("recv connect ip:", ip)
end

function handler.data()

end

skynet.start(function ()
    local listen_fd = socket.listen("0.0.0.0", 4000)
    socket.start(listen_fd, handler.accept)
end)