安装docker
如果安装最新版，直接输入如下命令即可

apt install docker-ce docker-ce-cli containerd.io

或者脚本安装docker官方脚本


curl -fsSL https://get.docker.com | bash -s docker


systemctl start docker

mkdir -p /opt/apps/openvpn


下载op
docker pull kylemanna/openvpn:2.4


docker run -v /opt/apps/openvpn:/etc/openvpn --rm kylemanna/openvpn:2.4 ovpn_genconfig -u tcp://公网IP



#生成密钥文件
docker run -v /opt/apps/openvpn:/etc/openvpn --rm -it kylemanna/openvpn:2.4 ovpn_initpki



输入私钥密码（输入时是看不见的）：
Enter PEM pass phrase:12345678


再输入一遍
Verifying - Enter PEM pass phrase:12345678
输入一个CA名称（我这里直接回车）
Common Name (eg: your user, host, or server name) [Easy-RSA CA]:
输入刚才设置的私钥密码（输入完成后会再让输入一次）
Enter pass phrase for /etc/openvpn/pki/private/ca.key:12345678







#生成客户端证书（这里的whsir改成你想要的名字）
docker run -v /opt/apps/openvpn:/etc/openvpn --rm -it kylemanna/openvpn:2.4 easyrsa build-client-full whsir nopass



再输入密码12345678



#导出客户端配置
mkdir -p /opt/apps/openvpn/conf
docker run -v /opt/apps/openvpn:/etc/openvpn --rm kylemanna/openvpn:2.4 ovpn_getclient whsir> /root/whsir.ovpn


客户端文件在root目录 whsir.ovpn

#启动OpenV服务
docker run --name openvpn -v /opt/apps/openvpn:/etc/openvpn -d -p 1194:1194/tcp --cap-add=NET_ADMIN kylemanna/openvpn:2.4
PS：
停止 openvpn
docker stop openvpn
启动 openvpn
docker start openvpn
#需要重启openvpn
docker restart openvpn


开机启动容器
命令：docker update --restart=always openvpn




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


./mproxy -l 8080

如果1194端口为openvpn

在客户端修改添加任意host伪装域名




remote v3-ml.douyinvod.com 80 tcp


http-proxy 这里填写代理ip 8080
            
