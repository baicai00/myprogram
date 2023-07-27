local skynet = require "skynet"
local protobuf = require "protobuf"

skynet.start(function ()
    local root = skynet.getenv("root") .. "pb/"
    protobuf.register_file(root .. "test.pb")

    local str = protobuf.encode("pb.Test", {name = "dengkai", sex="man"})
    local data = protobuf.decode("pb.Test", str)
    skynet.error("name=", data.name, "sex=", data.sex)

    skynet.exit()
end)