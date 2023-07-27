local beelog = require "beelog"


local function dump(root)
    local cache = {  [root] = "." }
    local function _dump(t,space,name)
        local temp = {}
        for k,v in pairs(t) do
            local key = tostring(k)
            if cache[v] then
                table.insert(temp," +" .. key .. " {" .. cache[v].."}")
            elseif type(v) == "table" then
                local new_key = name .. "." .. key
                cache[v] = new_key
                table.insert(temp," +" .. key .. _dump(v,space .. (next(t,k) and " |" or " " ).. string.rep(" ",#key),new_key))
            else
                table.insert(temp," " .. key .. " [" .. tostring(v).."]")
            end
        end
        return table.concat(temp,"\n"..space)
    end
    return "\n" .. _dump(root, "","")
end

do
    local _tostring = tostring
    tostring = function(v)
        if type(v) == 'table' then
            return dump(v)
        else
            return _tostring(v)
        end
    end
end

beelog.init()
local msg = {
    uid = 1,
    room_sub_type = 10,
}
beelog.error("test", "1111", tostring(msg))