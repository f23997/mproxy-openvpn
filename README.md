
openvpn免流
http connect代理把openvpn的流量伪装成http流量并修改host伪装任意域名

搭建教程
https://github.com/f23997/mproxy-openvpn/blob/main/搭建教程.txt


mproxy代理host伪装混淆域名 转接openvpn
这个代理和openvpn部署在同一个服务器












%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
这里提供openvpn镜像包
下载到linux

git clone https://github.com/f23997/mproxy-openvpn.git

进入目录
cd mproxy-openvpn

openvpn.tar为openvpn镜像包

通过sftp上传国内到主机


恢复镜像包命令


docker load -i openvpn.tar
