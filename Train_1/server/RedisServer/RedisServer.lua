local redis = require "resty.redis"
local Server = require "Server"
local SSRedis_pb = require "SSRedis_pb"

local ok, new_tab = pcall(require, "table.new")
if not ok then
    new_tab = function (narr, nrec) return {} end
end

local _M = new_tab(0, 5)

_M.new_tab = new_tab -- 为什么要加这一句???

local mt = { __index = _M }

function _M.new(self, host, port, password)
    local server, err = Server:new({timeout = 3000})
    if not server then
        return nil, err
    end

    local red = redis:new()
    red:set_timeout(1000) -- 1 sec

    local ok, err = red:connect(host, port)
    if not ok then
        return nil, err
    end

    if password and string.len(password) > 0 then
        -- 验证密码, todo
    end

    return setmetatable({
        server = server,
        red = red
    }, mt)
end

function _M.run(self)
    if not self.server then
       return nil, "server not initialized yet" 
    end

    if not self.red then
        return nil, "client not initialized yet"
    end

    while true do
        local data, err = self.server:recv()
        --处理接收到的数据 todo
    end
end

return _M
