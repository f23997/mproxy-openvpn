mproxy代理 转接openvpn
下载文件后

apt install gcc -y
centos用 yum 安装
gcc -DPORTS=443 -o mproxy mproxy.c

chmod +x mproxy


./mproxy -l 8080

如果443端口为openvpn

在客户端修改添加任意host伪装域名
remote v3-ml.douyinvod.com 80 tcp
http-proxy 这里填写代理ip 8080
            
