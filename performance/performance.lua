request= function()
local file = io.open("/home/baitianyu/projects/RPCFrame/performance/performance_params.txt", "r")
local body_data = file:read('*all')
file:close()
local data = body_data
wrk.method = "POST"
wrk.body   =string.format(data,tostring(uuid))
wrk.headers["Content-Type"] = "text/html;charset=utf-8"
wrk.path = "/method"
return wrk.format()
end