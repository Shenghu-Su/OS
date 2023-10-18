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
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <wait.h>

#define pf printf
#define pe perror

char cmd[1024] = {};

void StartNewPid(int client_fd, struct sockaddr_in* client_addr);
void ProcessRequest(int client_fd, struct sockaddr_in* client_addr);
void PrIP(struct sockaddr_in* client_addr);
void UpLoad(int client_fd);
void DownLoad(int client_fd);
void GetList(int client_fd, char *fileList);

int main() {
    pf("服务器启动，创建socket...\n");
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sfd) {
        pe("socket");
        return -1;
    }

    struct sockaddr_in saddr = {};
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(50000);
    saddr.sin_addr.s_addr = inet_addr("192.168.173.99");

    pf("socket绑定...\n");
    if (-1 == bind(sfd, (struct sockaddr *)&saddr, sizeof saddr)) {
        pe("bind");
        return -1;
    }

    pf("设置监听...\n");
    if (-1 == listen(sfd, 10)) {
    	// 最多允许10个客户端处于等待状态
        pe("listen");
        return -1;
    }

    pf("等待客户端连接...\n");
    while(true) {
        struct sockaddr_in caddr;
        socklen_t len = sizeof caddr;
        int cfd = accept(sfd, (struct sockaddr *)&caddr, &len);
        if (-1 == cfd) {
            pe("accept");
            continue;
        }
        char bufip[32] = {};
        inet_ntop(AF_INET, &caddr.sin_addr, bufip, sizeof bufip);
        pf("connect... IP:  %s\n", bufip);
        StartNewPid(cfd, &caddr);
    }
    close(sfd);
    return 0;
}

void StartNewPid(int client_fd, struct sockaddr_in* client_addr) {
    pid_t pid = fork();
    if (0 > pid) {
    	pe("fork");
    	return;
    }
    else if (0 == pid) {
    	if (fork() == 0) {
    	    ProcessRequest(client_fd, client_addr);
    	}
    	exit(0);
    }
    else {
    	close(client_fd);
    	waitpid(pid, NULL, 0);
    }
}

void ProcessRequest(int client_fd, struct sockaddr_in* client_addr) {
	char buf[1024] = {};
	while(true) {
		memset(buf, 0, sizeof buf);
		ssize_t c_size = read(client_fd, buf, sizeof buf);
		if (-1 == c_size) {
			pe("read");
			continue;
		}
		PrIP(client_addr);
		if (0 == strncmp(buf, "list", 4)) {
			pf("请求查询...\n");
			char *fileList = {0};
			GetList(client_fd, fileList);
		}
		else if (0 == strncmp(buf, "load", 4)) {
			pf("请求下载...\n");
			DownLoad(client_fd);
		}
		else if (0 == strncmp(buf, "upld", 4)) {
			pf("请求上传...\n");
			UpLoad(client_fd);
		}
		else if (0 == strncmp(buf, "over", 4)) {
			pf("当前用户退出...\n");
			close(client_fd);
			break;
		}
		else {
			strcpy(buf, "false: [ list | load | upld | over ]");
			write(client_fd, buf, sizeof buf);
		}
	}
}

void PrIP(struct sockaddr_in* client_addr) {
	pf("client %s :  ", inet_ntoa(client_addr->sin_addr));
}

// 上传
void UpLoad(int client_fd) {
	char buf[1024] = {};
	write(client_fd, "success", 8);
	
	read(client_fd, buf, sizeof buf);
	if (0 == strncmp(buf, "err", 5)) {
		pf("接受终止\n");
		return;
	}
	if (0 == strncmp(buf, "suc", 5)) {
		pf("success...准备接收\n");
	}
	
	char filename[64] = {};
	memset(filename, 0, sizeof filename);
	int f_size = read(client_fd, filename, 5);
	if (f_size == -1) {
		pe("read");
	}
	pf("%d\n", f_size);
	pf("上传文件: %s\n", filename);
	
	int fd = open(filename, O_CREAT | O_RDWR, 0777);
	int bytes = 0, ok = 0;
	while(true) {
		memset(buf, 0, sizeof buf);
		bytes = read(client_fd, buf, sizeof buf);
		// pf("debug.1\n");
		if (0 == strncmp(buf, "OVER", 4)) {
			// pf("debug.3\n");
			break;
		}
		pf("bytes: %d\n", bytes);
		write(fd, buf, bytes);
		// pf("debug.2\n");
		ok = 1;
	}
	if (ok) {
		pf("OVER!\n");
		write(client_fd, "success && over!", 17);
	}
	else {
		pf("ERROR!\n");
		write(client_fd, "error!", 7);
	}
	close(fd);
	return;
}

// 下载
void DownLoad(int client_fd) {
	char buf[1024] = {};
	write(client_fd, "sucess", 7);
	
	read(client_fd, buf, sizeof buf);
	if (0 == strncmp(buf, "err", 5)) {
		pf("取消下载\n");
		return;
	}
	if (0 == strncmp(buf, "suc", 5)) {
		pf("success...准备下载\n");
	}
	
	char *fileList = {0};
	GetList(client_fd, fileList);
	
	char filename[64] = {};
	memset(filename, 0, sizeof filename);
	int f_size = read(client_fd, filename, sizeof filename);
	if (-1 == f_size) {
		pe("read");
	}
	if (strstr(fileList, filename) == NULL) {
		pf("err: 文件名不存在，下载终止: %s\n", filename);
		write(client_fd, "error!", 7);
		return;
	}
	
	write(client_fd, "success && start!\n", 19);
	int fd = open(filename, O_RDONLY);
	int bytes = 0, ok = 0;
	while(true) {
		memset(buf, 0, sizeof buf);
		bytes = read(fd, buf, sizeof buf);
		if (0 == bytes) {
			break;
		}
		pf("bytes: %d\n", bytes);
		write(client_fd, buf, sizeof buf);
		ok = 1;
	}
	if (ok) {
		pf("OVER!\n");
		write(client_fd, "success && over!", 17);
	}
	else {
		pf("ERROR!\n");
		write(client_fd, "error!", 7);
	}
	close(fd);
	return;
}

/*
// 查询
void GetList(int client_fd, char *fileList) {
	system("ls >.DirTemp");
	int fd = open(".DirTemp", O_RDONLY, 0666);
	if (-1 == fd) {
		pe("open");
		return;
	}
	read(fd, fileList, sizeof(fileList) - 1);
	pf("当前目录列表: %s\n", fileList);
	close(fd);
    unlink(".DirTemp");
	send(client_fd, "NULL", 4, 0);
	return;
}
*/

void GetList(int client_fd, char *fileList) {
    char buf[1024] = {};
    DIR *dir = opendir("/");
    if(dir == NULL) {
        pe("opendir");
        return;
    }
    struct dirent* dt = NULL;
    while((dt == readdir(dir)) != NULL) {
        strcpy(buf, dt->d_name);
        strcat(fileList, buf);
        strcat(fileList, " ");
    }
    pf("fileList: %s\n", fileList);
    write(client_fd, fileList, strlen(fileList));
    closedir(dir);
}