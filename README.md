
openvpn免流
http connect代理把流量伪装成connect流量并修改host伪装任意域名,只要它的协议支持http-proxy的方式

编译
gcc -o mproxy mproxy.c -pthread

监听http connect端口80并转发到任意ip地址和tcp协议的端口
./mproxy -l 80 -r 127.0.0.1:1194 -d

ssh openvpn都支持代理


搭建教程
https://github.com/f23997/mproxy-openvpn/blob/main/搭建教程.txt


mproxy代理host伪装混淆域名 转接openvpn
这个代理和openvpn部署在同一个服务器












%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


