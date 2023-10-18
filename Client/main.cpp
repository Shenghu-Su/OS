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
	buffer[cnt] = '\0';
	return cnt - 1;
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
void init(){
	//服务器ip地址
	string ip  = "127.0.0.1";
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
	read2(fd, buf, sizeof buf);
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
	sleep(1);
	while(1){
		//接收文件
		int bytes =  read(fd, buf, sizeof buf);
		if(bytes == 0){
			break;
		}
		write(fld, buf, sizeof buf);
		sleep(1);
	}
	//关闭文件描述符 i
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
	sleep(1);
	write(fd, (path).c_str(), strlen((path).c_str()));
	sleep(1);
	while(1){
		//发送文件
		int bytes =  read(fld, buf, sizeof buf);
		cout << buf << endl;
		write(fd, buf, bytes);
		sleep(1);
		if(bytes <= 0){
			write(fd, "OVER", 4);
			cout << "exit with " << bytes << endl;
			break;
		}
		//write(fd, buf, bytes);
	}
	cout << "wait" << endl;
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
	}
	//关闭套接字文件描述符
	close(fd);
	return 0;
}
