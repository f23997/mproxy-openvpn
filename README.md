下载到linux

git clone https://github.com/f23997/mproxy-openvpn.git

进入目录
cd mproxy-openvpn

openvpn.tar为openvpn镜像包

通过sftp上传国内到主机


恢复镜像包命令


docker load -i openvpn.tar





搭建教程
https://github.com/f23997/mproxy-openvpn/blob/main/搭建教程.txt


mproxy代理host伪装混淆域名 转接openvpn
这个代理和openvpn部署在同一个服务器
下载文件

wget https://github.com/f23997/mproxy-openvpn/blob/17f9ca0bedc80c1748cf859a380a470c7a91fab8/mproxy.c.zip


解压文件
unzip mproxy.c.zip

apt install gcc -y



###centos用 yum 安装


gcc -o mproxy mproxy.c

chmod +x mproxy


./mproxy -l 80 -r 127.0.0.1:1194 -d

如果1194端口为openvpn

在客户端修改添加任意host伪装域名




remote v3-ml.douyinvod.com 80 tcp


http-proxy 这里填写代理ip 80
            
