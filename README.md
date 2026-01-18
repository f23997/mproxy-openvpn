OpenVPN HTTP camouflage / disguise
-
Normal proxies send CONNECT with the real IP and port.
Hoproxy sends CONNECT with any custom domain + port, then forwards to the specified real TCP address/port.
The client can request any arbitrary domain name.
In the place where you fill in the "remote IP" on the client side, you can actually put any domain name you want — it will be rewritten and forwarded to the specified IP:port.
SSH and OpenVPN both support this kind of proxy.
You can also use tools like proxytunnel.
Basically, any protocol that supports http-proxy can use this method.
User Guide https://github.com/f23997/openvpn-Free-internet/blob/main/%E6%90%AD%E5%BB%BA%E6%95%99%E7%A8%8B.txt
-
openvpn http伪装
Works in countries with internet censorship
Can be used in countries with internet restrictions
-

# -d          : Run hoproxy in the background (detached/daemon mode)
# -l 80    : Listen on local port 80
# -r 127.0.0.1:1194 : Forward all incoming connections to localhost:1194 (local OpenVPN port)

./hoproxy -d -l 80 -r 127.0.0.1:1194

在国内可用

一般代理会发送connect 真实ip和端口，hoproxy会发送connect任意自定义域名端口，转发到指定的地址tcp端口

客户端任意域名请求，在客户端填写远程ip的地方可以改写任意域名，转到指定ip端口
ssh openvpn都支持代理
也能搭配proxytunnel 
-
只要是支持http-proxy的协议都可以使用

搭建教程
https://github.com/f23997/openvpn-Free-internet/blob/main/%E6%90%AD%E5%BB%BA%E6%95%99%E7%A8%8B.txt















%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


