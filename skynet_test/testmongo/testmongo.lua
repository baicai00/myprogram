local skynet = require "skynet"
local mongo = require "skynet.db.mongo"

local function test_auth(conf)
    local c = mongo.client({host = conf.host, port = conf.port})
    local db = c[conf.authdb]
    db:auth(conf.username, conf.password)
end

local function create_client()
    return mongo.client({
        host = "127.0.0.1",
        port = 27017,
        username = "test",
        password = "123",
        authdb = "admin",
    })
end

local function test_insert_without_index()
    local c = create_client()
    local db = c["record"]
    local ok, err, ret = db.testcoll:safe_insert({test_key = 1})
	assert(ok and ret and ret.n == 1, err)

	ok, err, ret = db.testcoll:safe_insert({test_key = 1})
	assert(ok and ret and ret.n == 1, err)
end

local function test_insert_with_index()

end

local function test_find_and_remove()

end

local function test_runcommand()

end

local function test_expire_index()

end

local function test_safe_batch_insert()

end

local function test_safe_batch_delete()

end

skynet.start(function ()
    test_insert_without_index()
end)