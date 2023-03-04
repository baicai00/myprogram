local c = require "module.core_aa"

local ret1, ret2 = c.madd(1.1, 4) -- 调用module库的madd函数
print(ret1)
print(ret2)
print(c.msub(5, 1.2))