local ok, new_tab = pcall(require, "table.new")
if not ok then
    new_tab = function (narr, nrec) return {} end
end

local _M = new_tab(0, 5)

_M.new_tab = new_tab

local mt = { __index = _M } -- 这一句是什么意思???

function _M.new(self, opts)
    local sock, err = ngx.req.socket(true)
    if not sock then
        return nil, err
    end

    local max_data_len, timeout
    if opts then
        max_data_len = opts.max_data_len
        timeout = opts.timeout
        if timeout then
            sock:settimeout(timeout)
        end
    end

    return setmetatable({
        sock = sock,
        max_data_len = max_data_len or 131072
    }, mt)
end

function _M.recv(self)
    --接收数据 todo
end

funciton _M.send(self, data)
    --发送数据 todo
end

return _M