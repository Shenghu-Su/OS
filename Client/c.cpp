#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
using namespace std;
int fd;//套接字文件描述符
char buf[1024];//缓冲区

void init(){
	//服务器ip地址
	string ip  = "172.20.10.4";
	//服务器端口号
	int port = 50000;
	fd = socket(AF_INET,SOCK_STREAM, 0);
	if(fd == -1){
                cerr << "Create socket fail" << endl;
                exit(1);
        }		
	sockaddr_in sd;
	//指定类型为internet
	sd.sin_family = AF_INET;
	sd.sin_port = htons(port);
	sd.sin_addr.s_addr = inet_addr(ip.c_str());
	//发起连接
	if(connect(fd, (sockaddr *)&sd, sizeof sd) == -1){
		cerr << "Connect fail" << endl;
		exit(1);
	}
}
void sendlist(string command){
	//拷贝命令
	strcpy(buf, command.c_str());
	//发送命令 
	write(fd, buf, sizeof buf);
	read(fd, buf, sizeof buf);
	cout << buf << endl;
}
void sendload(string command){
	//解析下载路径
	string path = command.substr(5, (int)command.size() - 5);
	int fld = open(path.c_str(), O_CREAT | O_RDWR, 0777);
	if(fld == -1){
		cerr << "Open file fial(Load)" << endl;
		exit(1);
	}
	strcpy(buf, command.substr(0,4).c_str());
	write(fd, buf, strlen(buf));
	read(fd, buf, sizeof buf);
	cout << "response:" << buf << endl;
	strcpy(buf, "suc\0");
	write(fd, buf, 4);
	write(fd, path.c_str(), strlen((path).c_str()));
	while(1){
		//接收文件
		sleep(1);
		int bytes = read(fd, buf, sizeof buf);
		if(0 == strncmp(buf, "OVER", 4)){
			break;
		}
		cout << "bytes:" << bytes << endl;
		write(fld, buf, bytes);
	}
	sleep(1);
	read(fd, buf, sizeof buf);
	cout << "response:" << buf << endl;
	//关闭文件描述符
	close(fld);
}
void sendupld(string command){	
	//解析下载路径
	string path = command.substr(5, (int)command.size() - 5);
	int fld = open(path.c_str(), O_RDONLY);
	if(fld == -1){
		cerr << "Open file fial(Load)" << endl;
		exit(1);
	}
	//发送命令
	strcpy(buf, command.substr(0,4).c_str());
	write(fd, buf, strlen(buf));
	read(fd, buf, sizeof buf);
	cout << "response:" << buf << endl;
	strcpy(buf, "suc\0");
	write(fd, buf, 4);
	write(fd, path.c_str(), strlen((path).c_str()));
	while(1){
		//发送文件
		sleep(1);
		int bytes = read(fld, buf, sizeof buf);
		write(fd, buf, bytes);
		if(bytes <= 0){
			strcpy(buf, "OVER");
			write(fd, buf, 4);
			break;
		}
		// cout << buf << endl;
	}
	sleep(1);
	read(fd, buf, sizeof buf);
	cout << "response:" << buf << endl;
	//关闭文件描述符
	close(fld);
}
int main(){
	//初始化套接字，连接到服务器
	init();
	string command;
	while(1){
		getline(cin, command);
		memset(buf, 0, sizeof buf);
		//解析出操作字段
		string op = command.substr(0, 4);
		if(op == "list"){
			sendlist(command);
		}
		else if(op == "load"){
			sendload(command);
		}
		else if(op == "upld"){
			sendupld(command);
		}
		else if(op == "quit"){
			break;
		}
		else if(op == "over"){
			write(fd, "over", 4);
		}
		cout << endl;
	}
	//关闭套接字文件描述符
	close(fd);
	return 0;
}