
openvpn免流篇



代理从客户端获取任意域名请求，转到指定ip端口

编译
gcc -o mproxy mproxy.c -pthread


./mproxy -l 80 -r 127.0.0.1:1194 -d

ssh openvpn都支持代理


搭建教程
https://github.com/f23997/mproxy-openvpn/blob/main/搭建教程.txt




ssh篇
termux在里面
安装corkscrew openssh

手机socksdroid连接本机2080端口，全局流量放行com.termux


termux里面运行
ssh -o ProxyCommand="corkscrew 代理服务器 80 %h %p" -v -D 2080 root@混淆域名 -p 80

ssh -o ProxyCommand="corkscrew 代理ip 80 %h %p" -v -D 2080 root@qq.com -p 80

服务器运./mproxy -l 80 -r 127.0.0.1:22 -d









%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


