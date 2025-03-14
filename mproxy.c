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
#include <sys/epoll.h>

#define VERSION "1.0"
#define INITIAL_BUFFER_SIZE 4096
#define MAX_HOST_LEN 256
#define MAX_EVENTS 1024

// 全局变量
int listen_port = 0;
char remote_host[256] = {0};
int remote_port = 0;
int server_mode = 0;
int client_mode = 0;
char xor_key[256] = {0};
int daemon_mode = 0;
char fake_host[256] = {0};

// 连接对结构体
typedef struct {
    int client_fd;
    int remote_fd;
} ConnectionPair;

// 打印使用说明
void usage(const char *prog) {
    fprintf(stderr, "Usage: %s -l <listen_port> [-r <remote_host:port>] [-s|-c] [-k <key>] [-d] [-f <fake_host>]\n", prog);
    fprintf(stderr, "  -l <port>       Listen port\n");
    fprintf(stderr, "  -r <host:port>  Remote host and port (e.g., 127.0.0.1:1194)\n");
    fprintf(stderr, "  -s             Server mode (placeholder)\n");
    fprintf(stderr, "  -c             Client mode (placeholder)\n");
    fprintf(stderr, "  -k <key>       XOR key (ignored)\n");
    fprintf(stderr, "  -d             Daemon mode\n");
    fprintf(stderr, "  -f <host>      Fake Host for HTTP CONNECT response (optional)\n");
    exit(1);
}

// 解析命令行参数
void parse_args(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "l:r:scdk:f:")) != -1) {
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
            case 'f':
                strncpy(fake_host, optarg, sizeof(fake_host) - 1);
                break;
            default:
                usage(argv[0]);
        }
    }
    if (listen_port == 0 || remote_port == 0) {
        usage(argv[0]);
    }
}

// 检查是否为 IP 地址
int is_ip_address(const char *str) {
    struct in_addr addr;
    return inet_pton(AF_INET, str, &addr) == 1;
}

// 处理新客户端连接
int handle_new_connection(int client_fd, int epoll_fd) {
    char buffer[INITIAL_BUFFER_SIZE];
    ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        close(client_fd);
        return -1;
    }
    buffer[n] = '\0';

    // 提取 CONNECT 请求中的目标地址
    char target_host[MAX_HOST_LEN] = {0};
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

    // 确定伪装的 Host
    char final_host[MAX_HOST_LEN] = {0};
    if (strlen(target_host) > 0) {
        if (is_ip_address(target_host) && strlen(fake_host) > 0) {
            strncpy(final_host, fake_host, sizeof(final_host) - 1);
        } else {
            strncpy(final_host, target_host, sizeof(final_host) - 1);
        }
    } else {
        if (strlen(fake_host) > 0) {
            strncpy(final_host, fake_host, sizeof(final_host) - 1);
        } else if (strlen(remote_host) > 0) {
            strncpy(final_host, remote_host, sizeof(final_host) - 1);
        } else {
            strcpy(final_host, "default.example.com");
        }
    }

    // 发送 HTTP CONNECT 响应
    char response[512];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 Connection Established\r\n"
             "Host: %s\r\n"
             "Connection: keep-alive\r\n\r\n", final_host);
    write(client_fd, response, strlen(response));

    // 连接远程服务器
    int remote_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (remote_fd < 0) {
        close(client_fd);
        return -1;
    }
    struct sockaddr_in remote_addr = { .sin_family = AF_INET, .sin_port = htons(remote_port) };
    inet_pton(AF_INET, remote_host, &remote_addr.sin_addr);
    if (connect(remote_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
        close(client_fd);
        close(remote_fd);
        return -1;
    }

    // 设置非阻塞
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    fcntl(remote_fd, F_SETFL, O_NONBLOCK);

    // 添加到 epoll
    struct epoll_event ev = { .events = EPOLLIN };
    ev.data.ptr = malloc(sizeof(ConnectionPair));
    ((ConnectionPair *)ev.data.ptr)->client_fd = client_fd;
    ((ConnectionPair *)ev.data.ptr)->remote_fd = remote_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
    ev.data.ptr = malloc(sizeof(ConnectionPair));
    ((ConnectionPair *)ev.data.ptr)->client_fd = client_fd;
    ((ConnectionPair *)ev.data.ptr)->remote_fd = remote_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, remote_fd, &ev);

    return 0;
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    // 启用守护进程模式
    if (daemon_mode) {
        if (daemon(0, 0) < 0) {
            perror("daemon");
            exit(1);
        }
    }

    signal(SIGPIPE, SIG_IGN);

    // 创建监听 socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in server_addr = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(listen_port) };
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(listen_fd, 128) < 0) {
        perror("listen");
        exit(1);
    }

    // 启用 TCP Fast Open
    int tfo = 1;
    setsockopt(listen_fd, IPPROTO_TCP, TCP_FASTOPEN, &tfo, sizeof(tfo));

    // 创建 epoll 实例
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        exit(1);
    }
    struct epoll_event ev = { .events = EPOLLIN, .data.fd = listen_fd };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    // 事件循环
    struct epoll_event events[MAX_EVENTS];
    if (!daemon_mode) {
        printf("mproxy %s listening on port %d\n", VERSION, listen_port);
    }
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                int client_fd = accept(listen_fd, NULL, NULL);
                if (client_fd < 0) continue;
                fcntl(client_fd, F_SETFL, O_NONBLOCK);
                if (handle_new_connection(client_fd, epoll_fd) < 0) {
                    close(client_fd);
                }
            } else {
                ConnectionPair *pair = (ConnectionPair *)events[i].data.ptr;
                int fd = events[i].data.fd;
                int target_fd = (fd == pair->client_fd) ? pair->remote_fd : pair->client_fd;

                char *buffer = malloc(INITIAL_BUFFER_SIZE);
                size_t buf_size = INITIAL_BUFFER_SIZE;
                ssize_t n = read(fd, buffer, buf_size);
                if (n <= 0) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pair->client_fd, NULL);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pair->remote_fd, NULL);
                    close(pair->client_fd);
                    close(pair->remote_fd);
                    free(pair);
                } else {
                    while (n == buf_size) {
                        buf_size *= 2;
                        buffer = realloc(buffer, buf_size);
                        ssize_t extra = read(fd, buffer + n, buf_size - n);
                        if (extra <= 0) break;
                        n += extra;
                    }
                    write(target_fd, buffer, n);
                }
                free(buffer);
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);
    return 0;
}