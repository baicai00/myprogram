local skynet = require "skynet"
local mongo = require "skynet.db.mongo"

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

local function test_batch_insert(client)
    local db = client["record"]
    local docs = {
        [1] = {uid = 1, name = "Tom"},
        [2] = {uid = 2, name = "Jerry"},
        [3] = {uid = 1, name = "Rose"},
        [4] = {uid = 2, name = "Jim"},
    }
    db.testcoll:batch_insert(docs)
end

-- single为true表示只删除匹配到的第一条数据
local function test_safe_batch_delete(client, single)
    local db = client["record"]
    local deletes = {
        [1] = {uid = 1},
        [2] = {name = "Jim"},
    }
    local ok, err, r = db.testcoll:safe_batch_delete(deletes, single)
    if not ok then
        print(tostring(err))
        print(tostring(r))
    else
        print(tostring(ok))
    end
end

local function test_find(client)
    local db = client["record"]
    local query = {uid = 1}
    local projection = {_id = 0, name = 1}
    local cursor = db.testcoll:find(query, projection)
    while cursor:hasNext() do
        local row = cursor:next()
        print(row["name"])
    end
end

local function test_aggregate(client)
    local db = client["record"]

    -- 删除所有文档
    db.testcoll:delete({}, false)

    db.testcoll:insert({uid = 931, room_sub_type = 20, profit = 1000})
    db.testcoll:insert({uid = 931, room_sub_type = 21, profit = 6000})
    db.testcoll:insert({uid = 931, room_sub_type = 21, profit = 2000})
    db.testcoll:insert({uid = 931, room_sub_type = 23, profit = 5000})
    db.testcoll:insert({uid = 932, room_sub_type = 20, profit = 3000})
    db.testcoll:insert({uid = 933, room_sub_type = 21, profit = 4000})

    local pipeline = {
        {
            ["$match"] = {
                uid = 931,
                room_sub_type = {["$in"] = {20, 21}},
            }
        },
        {
            ["$group"] = {
                _id = "$room_sub_type",
                total = {["$sum"] = "$profit"},
            }
        }
    }
    local ret = db.testcoll:aggregate(pipeline, {cursor={}})
    --local ret = db:runCommand("aggregate", "testcoll", "pipeline", pipeline, "cursor", {})
    print(tostring(ret))
end

skynet.start(function ()
    local client = create_client()
    -- test_insert_without_index(client)
    -- test_batch_insert(client)
    -- skynet.sleep(20 * 100)
    -- print("=======start delete")
    -- test_safe_batch_delete(client, true)
    -- test_safe_batch_delete(client, false)
    -- test_find(client)

    -- test_aggregate(client)
end)