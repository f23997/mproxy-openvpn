搭建教程
https://github.com/f23997/mproxy-openvpn/blob/main/openvpn免流搭建.txt


mproxy代理host伪装混淆域名 转接openvpn
这个代理和openvpn部署在同一个服务器
下载文件

wget https://github.com/f23997/abc123/files/14479919/mproxy.c.zip


解压文件
unzip mproxy.c.zip

apt install gcc -y



###centos用 yum 安装


gcc -DPORTS=1194 -o mproxy mproxy.c

chmod +x mproxy


./mproxy -l 80

如果1194端口为openvpn

在客户端修改添加任意host伪装域名




remote v3-ml.douyinvod.com 80 tcp


http-proxy 这里填写代理ip 8080
            
