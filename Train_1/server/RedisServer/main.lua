local server, err = require("RedisServer"):new("127.0.0.1", 6379)
if not server then
    ngx.log(ngx.ERR, "Failed to new RedisServer: ", err)
else
    ngx.log(ngx.INFO, "New RedisServer")
    server:run()
end