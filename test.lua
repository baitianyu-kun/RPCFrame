request= function()
local file = io.open("/home/baitianyu/rpc/RPCFrame/test_res.txt", "r")
local body_data = file:read('*all')
file:close()
local data = body_data
wrk.method = "POST"
wrk.body   =string.format(data,tostring(uuid))
wrk.headers["Content-Type"] = "text/html;charset=utf-8"
return wrk.format()
end