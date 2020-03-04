local server = require "resty.websocket.server2"

local wb, err = server:new{
	timeout = 5000,
	max_payload_len = 65535,
}

if not wb then
	ngx.log(ngx.ERR, "failed to new websocket: ", err)
	return ngx.exit(444)
end

while true do
	local data, typ, err = wb:recv_frame()
	if not data then
		if not string.find(err, "timeout", 1, true) then
			ngx.log(ngx.ERR, "failed to receive a frame: ", err)
			return ngx.exit(444)
		end
	end

	if typ == "text" then
		ngx.log(ngx.ERR, "recv: ", data)
		wb:send_text(data)
		return ngx.exit(0)
	end

	
end
