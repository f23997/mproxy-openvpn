#mproxy


apt install gcc -y

gcc -DPORTS=443 -o mproxy mproxy.c

chmod +x mproxy


./mproxy -l 8080 -h 127.0.0.1 443

443端口为openvpn
