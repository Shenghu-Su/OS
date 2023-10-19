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
void GetList(int client_fd);

int read2(int fild, char* buffer, int sz){
	int cnt = 0;
	while(cnt < sz){
		//获取返回
		int bt = read(fild, buffer + cnt, 1);
		if(bt == -1){
			return -1;
		}
		//输出
		int cp = 0;
		while(bt--){
			if(buffer[cnt] == '#'){
				cp = 1;
				break;
			}
			cnt++;
		}
		if(cp)break;
	}
	return cnt;
}

int write2(int flid, char* buffer, int sz){
	int bt = write(flid, buffer, sz);
	if(bt == -1){
		return -1;
	}
	bt = write(flid, "#", 1);
	if(bt == -1){
		return -1;
	}
	return sz;
}

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
    saddr.sin_addr.s_addr = inet_addr("172.20.10.4");

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
		pf("\n");
		memset(buf, 0, sizeof buf);
		ssize_t c_size = read(client_fd, buf, sizeof buf);
		if (-1 == c_size) {
			pe("read");
			continue;
		}
		PrIP(client_addr);
		if (0 == strncmp(buf, "list", 4)) {
			pf("请求查询...\n");
			GetList(client_fd);
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
	pf("上传文件: %s\n", filename);
	
	int fd = open(filename, O_CREAT | O_RDWR, 0777);
	int bytes = 0, ok = 0;
	while(true) {
		memset(buf, 0, sizeof buf);
		bytes = read(client_fd, buf, sizeof buf);
		if (0 == strncmp(buf, "OVER", 4)) {
			break;
		}
		pf("bytes: %d\n", bytes);
		write(fd, buf, bytes);
		ok = 1;
	}
	sleep(1);
	if (ok) {
		pf("download : success && over!\n");
		write(client_fd, "over!", 17);
	}
	else {
		pf("download : error!\n");
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
	if (0 == strncmp(buf, "err", 4)) {
		pf("取消下载\n");
		return;
	}
	if (0 == strncmp(buf, "suc", 4)) {
		pf("success...准备下载\n");
	}
	
	char list[1024] = {};
    DIR *dir = opendir(".");
    if(dir == NULL) {
        pe("opendir");
        return;
    }
    struct dirent* dt = NULL;
    while((dt = readdir(dir)) != NULL) {
        strcat(list, dt->d_name);
        strcat(list, " ");
    }
	
	char filename[64] = {};
	memset(filename, 0, sizeof filename);
	int f_size = read(client_fd, filename, sizeof filename);
	if (-1 == f_size) {
		pe("read");
	}
	if (strstr(list, filename) == NULL) {
		pf("err: 文件名不存在，下载终止: %s\n", filename);
		write(client_fd, "error!", 7);
		return;
	}
	
	write(client_fd, "start!", 7);
	int fd = open(filename, O_RDONLY);
	int bytes = 0, ok = 0;
	while(true) {
		memset(buf, 0, sizeof buf);
		sleep(1);
		bytes = read(fd, buf, sizeof buf);
		write(client_fd, buf, bytes);
		if (bytes <= 0) {
			strcpy(buf, "OVER");
			write(client_fd, buf, 4);
			break;
		}
		pf("bytes: %d\n", bytes);
		ok = 1;
	}
	sleep(1);
	if (ok) {
		pf("download : success && over!\n");
		write(client_fd, "over!", 17);
	}
	else {
		pf("download : error!\n");
		write(client_fd, "error!", 7);
	}
	close(fd);
	return;
}

// 查询
void GetList(int client_fd) {
    char buf[1024] = {};
    DIR *dir = opendir(".");
    if(dir == NULL) {
        pe("opendir");
        return;
    }
    struct dirent* dt = NULL;
    while((dt = readdir(dir)) != NULL) {
        strcat(buf, dt->d_name);
        strcat(buf, "\n");
    }
    write(client_fd, buf, strlen(buf) + 1);
    pf("getlist : success && over!\n");
    closedir(dir);
    return;
}