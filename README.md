## 参考[rocket](https://github.com/Gooddbird/rocket)实现的RPC框架

### 1. RPCServer服务端

### 2. RPCClient客户端

### 3. RegisterCenter注册中心

#### 3.1 RPCServer启动时会自动向注册中心注册service以及server ip和port，RPCClient启动时会从注册中心请求service，注册中心根据负载均衡规则返回提供服务的server ip和port

#### 3.2 注册中心会自动定时向服务器检测心跳，更新本地存储的service信息，超时则自动删除

#### 3.3 一致性Hash实现负载均衡

## Ref
https://github.com/Gooddbird/rocket

https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

https://zhuanlan.zhihu.com/p/379724672
