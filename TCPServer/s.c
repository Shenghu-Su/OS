#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>

#define pr printf
#define pe perror

char cmd[1024] = {};

// 服务启动
void start(int *cfd);
// 查询
void getlist(int *cfd);
// 下载
void download(int *cfd);
// 上传
void upload(int *cfd);

int main() {
    pr("服务器启动，创建socket...\n");
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sfd) {
        pe("socket");
        return -1;
    }

    struct sockaddr_in saddr = {};
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = inet_addr("172.19.85.161");
    socklen_t len = sizeof(saddr);

    pr("socket绑定...\n");
    if (-1 == bind(sfd, (struct sockaddr *)&saddr, len)) {
        pe("bind");
        return -1;
    }

    pr("设置监听...\n");
    if (-1 == listen(sfd, 10)) {
        pe("listen");
        return -1;
    }

    pr("等待客户端连接...\n");
    while(true) {
        struct sockaddr_in caddr = {};
        int cfd = accept(sfd, NULL, NULL);
        if (-1 == cfd) {
            pe("accept");
            continue;
        }
        pr("连接成功！\n");
        start(&cfd);
    }
    return 0;
}

// 启动
void start(int *cfd) {
    // while(true) {
        int c_size = read(*cfd, cmd, sizeof(cmd));
        if (-1 == c_size) {
            pe("read");
        }
        if (0 == strncmp(cmd, "list", 4)) {
            // 查询
            pr("客户请求查询...\n");
            getlist(cfd);
        }
        else if (0 == strncmp(cmd, "load", 4)) {
            // 下载
            pr("客户请求下载...\n");
            download(cfd);
        }
        else if (0 == strncmp(cmd, "upld", 4)) {
            // 上传
            pr("客户请求上传...\n");
            upload(cfd);
        }
        else if (0 == strncmp(cmd, "quit", 4)) {
            // 退出
            pr("收到退出指令，运行结束\n");
            return;
        }
    // }
}

// 查询
void getlist(int *cfd) {
    char buf[1024] = {};
    char path[1024] = {};
    /*
    strcpy(path, ".");
    while(true) {
        read(*cfd, &buf, sizeof(buf));
        puts(buf);
        if (0 == strncmp(buf, "quit", 4)) {
            // write(*cfd, &buf, strlen(buf);
            break;
        }
        // TODO
    }
    */
    strcpy(buf, "accept");
    write(*cfd, &buf, sizeof buf);
}

// 下载
void download(int *cfd) {
    char buf[1024] ={};
    char path[1024] = {};
    read(*cfd, &path, sizeof(path));
    int fd = open(path, O_RDONLY);
    if (-1 == fd) {
        pe("open");
        return;
    }
    int bytes = 0;
    while(1) {
        bytes = read(fd, &buf, sizeof(buf));
        if (0 == bytes) break;
        write(*cfd, &buf, bytes);
    }
    strcpy(buf, "OVER!");
    bytes = write(*cfd, &buf, strlen(buf));
    close(fd);
}

// 上传
void upload(int *cfd) {
    char buf[1024]= {};
    char path[1024] = {};
    read(*cfd, &path, sizeof(path));
    int fd = open(path, O_CREAT | O_EXCL | O_WRONLY);
    int bytes = 0;
    if (-1 == fd) {
        pr("open");
        return;
    }
    while(1) {
        bytes = read(fd, &buf, sizeof(buf));
        pr("read: %d\n", bytes);
        if (0 == bytes) break;
        write(*cfd, &buf, bytes);
    }
    strcpy(buf, "OVER!");
    bytes = write(*cfd, &buf, strlen(buf));
    close(fd);
}
