#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#define VERSION "1.0"
#define BUFFER_SIZE 8192
#define MAX_HOST_LEN 256

// 全局变量
int listen_port = 0;
char remote_host[256] = {0};
int remote_port = 0;
int server_mode = 0;
int client_mode = 0;
char xor_key[256] = {0};
int daemon_mode = 0;

// 打印使用说明
void usage(const char *prog) {
    fprintf(stderr, "Usage: %s -l <listen_port> [-r <remote_host:port>] [-s|-c] [-k <key>] [-d]\n", prog);
    fprintf(stderr, "  -l <port>       Listen port\n");
    fprintf(stderr, "  -r <host:port>  Remote host and port (e.g., 127.0.0.1:1194)\n");
    fprintf(stderr, "  -s             Server mode\n");
    fprintf(stderr, "  -c             Client mode\n");
    fprintf(stderr, "  -k <key>       XOR key for obfuscation\n");
    fprintf(stderr, "  -d             Daemon mode\n");
    exit(1);
}

// 解析命令行参数
void parse_args(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "l:r:scdk:")) != -1) {
        switch (opt) {
            case 'l':
                listen_port = atoi(optarg);
                break;
            case 'r': {
                char *colon = strchr(optarg, ':');
                if (colon) {
                    *colon = '\0';
                    strncpy(remote_host, optarg, sizeof(remote_host) - 1);
                    remote_port = atoi(colon + 1);
                } else {
                    strncpy(remote_host, optarg, sizeof(remote_host) - 1);
                    remote_port = 80;
                }
                break;
            }
            case 's':
                server_mode = 1;
                break;
            case 'c':
                client_mode = 1;
                break;
            case 'k':
                strncpy(xor_key, optarg, sizeof(xor_key) - 1);
                break;
            case 'd':
                daemon_mode = 1;
                break;
            default:
                usage(argv[0]);
        }
    }
    if (listen_port == 0) {
        usage(argv[0]);
    }
}

// XOR 加密/解密
void xor_data(char *data, size_t len) {
    if (strlen(xor_key) == 0) return;
    for (size_t i = 0; i < len; i++) {
        data[i] ^= xor_key[i % strlen(xor_key)];
    }
}

// 处理客户端连接
void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    char target_host[MAX_HOST_LEN] = {0};
    int remote_fd = -1;

    // 读取客户端请求
    ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        close(client_fd);
        return;
    }
    buffer[n] = '\0';

    // 提取 CONNECT 请求中的目标域名
    if (strncmp(buffer, "CONNECT", 7) == 0) {
        char *host_start = strchr(buffer, ' ') + 1;
        char *host_end = strchr(host_start, ':');
        if (host_end) {
            size_t host_len = host_end - host_start;
            if (host_len < MAX_HOST_LEN) {
                strncpy(target_host, host_start, host_len);
                target_host[host_len] = '\0';
            }
        }
    }

    // 如果未提取到域名，使用默认值或 remote_host
    if (strlen(target_host) == 0) {
        if (strlen(remote_host) > 0) {
            strncpy(target_host, remote_host, sizeof(target_host) - 1);
        } else {
            strcpy(target_host, "default.example.com");
        }
    }

    // 构造伪装的 HTTP CONNECT 响应
    char response[512];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 Connection Established\r\n"
             "Host: %s\r\n"
             "Connection: keep-alive\r\n\r\n", target_host);
    write(client_fd, response, strlen(response));

    // 连接到远程服务器（OpenVPN）
    struct sockaddr_in remote_addr;
    remote_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (remote_fd < 0) {
        perror("socket");
        close(client_fd);
        return;
    }

    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    inet_pton(AF_INET, remote_host, &remote_addr.sin_addr);

    if (connect(remote_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
        perror("connect");
        close(client_fd);
        close(remote_fd);
        return;
    }

    // 设置非阻塞
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    fcntl(remote_fd, F_SETFL, O_NONBLOCK);

    // 双向转发
    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(client_fd, &readfds);
        FD_SET(remote_fd, &readfds);
        int max_fd = (client_fd > remote_fd ? client_fd : remote_fd) + 1;

        if (select(max_fd, &readfds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (FD_ISSET(client_fd, &readfds)) {
            n = read(client_fd, buffer, sizeof(buffer));
            if (n <= 0) break;
            xor_data(buffer, n);
            write(remote_fd, buffer, n);
        }

        if (FD_ISSET(remote_fd, &readfds)) {
            n = read(remote_fd, buffer, sizeof(buffer));
            if (n <= 0) break;
            xor_data(buffer, n);
            write(client_fd, buffer, n);
        }
    }

    close(client_fd);
    close(remote_fd);
}

// 主函数
int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    if (daemon_mode) {
        if (daemon(0, 0) < 0) {
            perror("daemon");
            exit(1);
        }
    }

    signal(SIGPIPE, SIG_IGN);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(listen_port);

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(listen_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("mproxy %s listening on port %d\n", VERSION, listen_port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_fd);
            continue;
        } else if (pid == 0) {
            close(listen_fd);
            handle_client(client_fd);
            exit(0);
        } else {
            close(client_fd);
        }
    }

    close(listen_fd);
    return 0;
}