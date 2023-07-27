
function Test_load()
    local config_name = "simple.file"
    local f = assert(io.open(config_name, "r"))
    local source = f:read("*a")
    f:close()

    local tmp = {}
    assert(load(source, "@" .. config_name, "t", tmp))()
    print(tmp.test1) -- 读取到simple.file中的test1
    print(tmp.test2)
end


function Test_string_sub()
    local str = "hello"
    print(string.sub(str, 1, 2)) -- 输出："he"
end

Test_string_sub()