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
	string ip  = "192,168,0,0";
	//服务器端口号
	int port = 8080;
	fd = socket(AF_INET,SOCK_STREAM, 0);
	if(fd == -1){
                cerr << "Create socket fail" << endl;
                exit(1);
        }		
	sockaddr_in sd;
	//指定类型为internet
	sd.sin_family = AF_INET;
	sd.sin_port = port;
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
	//获取返回
	read(fd, buf, sizeof buf);
	//输出返回
	cout << buf << endl;
}
void sendload(string command){
	//解析下载路径
	string path = command.substr(5, (int)command.size() - 5);
	int fld = open(path.c_str(), O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	if(fld == -1){
		cerr << "Open file fial(Load)" << endl;
		exit(1);
	}
	//发送命令
	strcpy(buf, command.c_str());
	write(fd, buf, sizeof buf);
	while(1){
		//接收文件
		int bytes =  read(fd, buf, sizeof buf);
		if(bytes == 0){
			break;
		}
		write(fld, buf, sizeof buf);
	}
	//关闭文件描述符 
	close(fld);
}
void sendupld(string command){	
	//解析下载路径
	string path = command.substr(5, (int)command.size() - 5);
	int fld = open(path.c_str(), O_WRONLY);
	if(fld == -1){
		cerr << "Open file fial(Load)" << endl;
		exit(1);
	}
	//发送命令
	strcpy(buf, command.c_str());
	write(fd, buf, sizeof buf);
	while(1){
		//发送文件
		int bytes =  read(fld, buf, sizeof buf);
		if(bytes == 0){
			break;
		}
		write(fd, buf, sizeof buf);
	}
	//关闭文件描述符
	close(fld);
}
int main(){
	//初始化套接字，连接到服务器
	init();
	string command;
	while(1){
		getline(cin, command);
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
			exit(0);
		}
	}
	//关闭套接字文件描述符
	close(fd);
	return 0;
}
