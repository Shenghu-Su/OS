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
#define BUF_SIZE 1024
int fd;//套接字文件描述符
char buf[BUF_SIZE + 1];//缓冲区
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
//	return cnt - 1;
//	fix bug
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
//write
void wSessionL(string s, int bytes){
	s += ".sessionL";
	char buffer[BUF_SIZE + 1];
	memset(buffer, 0, sizeof buffer);
	int fld = open(s.c_str(), O_CREAT | O_WRONLY, 0777);
	if(fld == -1){
		cerr << "write session fail" << endl;
		return;
	}
	sprintf(buffer, "%d", bytes);
	write(fld, buffer, strlen(buffer));
	close(fld);
}
//del
void dSessionL(string s){
	s += ".sessionL";
	cout << "del session" << s << endl;
	remove(s.c_str());
}
//read
int rSessionL(string s){
	s += ".sessionL";
	char buffer[BUF_SIZE + 1];
	memset(buffer, 0, sizeof buffer);
	int fld = open(s.c_str(), O_RDONLY);
	if(fld == -1){
		return 0;
	}
	int stat = read(fld, buffer, sizeof(buffer) - 1);
	if(stat <= 0){
		return 0;
	}
	int bts;
	sscanf(buffer, "%d", &bts);
	close(fld);
       	return bts;
}
//write
void wSessionU(string s, int bytes){
	s += ".sessionU";
	char buffer[BUF_SIZE + 1];
	memset(buffer, 0, sizeof buffer);
	int fld = open(s.c_str(), O_CREAT | O_WRONLY, 0777);
	if(fld == -1){
		cerr << "write session fail" << endl;
		return;
	}
	sprintf(buffer, "%d", bytes);
	write(fld, buffer, strlen(buffer));
	close(fld);
}
//del
void dSessionU(string s){
	s += ".sessionU";
	cout << "del session" << s << endl;
	remove(s.c_str());
}
//read
int rSessionU(string s){
	s += ".sessionU";
	char buffer[BUF_SIZE + 1];
	memset(buffer, 0, sizeof buffer);
	int fld = open(s.c_str(), O_RDONLY);
	if(fld == -1){
		return 0;
	}
	int stat = read(fld, buffer, sizeof(buffer) - 1);
	if(stat <= 0){
		return 0;
	}
	int bts;
	sscanf(buffer, "%d", &bts);
	close(fld);
       	return bts;
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
	read(fd, buf, sizeof buf);
	cout << buf << endl;
}
void senddel(string command){
	//解析下载路径
	string path = command.substr(4, (int)command.size() - 4);
	strcpy(buf, command.substr(0,3).c_str());
	write(fd, buf, strlen(buf));
	read(fd, buf, sizeof buf);
	cout << "response:" << buf << endl;
	//strcpy(buf, "suc\0");
	//write(fd, buf, 4);
	write(fd, path.c_str(), strlen((path).c_str()));
//	sleep(1);
//	cout << "response:" << buf << endl;
	cout << "del success" << endl;
	//关闭文件描述符
}
void sendload(string command){
	//解析下载路径
	string path = command.substr(5, (int)command.size() - 5);
	strcpy(buf, command.substr(0,4).c_str());
	write(fd, buf, strlen(buf));
	read(fd, buf, sizeof buf);
	cout << "response:" << buf << endl;
	//发送offset，收到success
	int ofst = rSessionL(path);
	//创建文件
	int fld;
	if(ofst == 0){
		fld = open(path.c_str(), O_TRUNC | O_WRONLY, 0777);
	}
	else{
		fld = open(path.c_str(), O_CREAT | O_WRONLY, 0777);
	}
	if(fld == -1){
		cerr << "Open file fial(Load)" << endl;
		exit(1);
	}
	write(fd, to_string(ofst).c_str(), to_string(ofst).size());
	//sleep(1);
	read(fd, buf, sizeof buf);
	//
	strcpy(buf, "suc\0");
	write(fd, buf, 4);
	write(fd, path.c_str(), strlen((path).c_str()));
	read(fd, buf, sizeof buf);//收到start或者err
	if(strcmp(buf, "error!") == 0){
		cerr << buf << endl;
		return;
	}	
	//偏移
	int ot = lseek(fld, ofst, SEEK_SET);
	if(ot == -1){
		cerr << "偏移出错" << endl;
	}
	write(fd, "suc\0", 4);
	while(1){
		//接收文件
		int bytes = read(fd, buf, BUF_SIZE + 1);
		cout << "bytes:" << bytes << endl;
		write(fld, buf + 1, BUF_SIZE);
		//更新偏移量
		ofst += bytes - 1;
		wSessionL(path, ofst);
		if(buf[0] == '0')break;
	}
	read(fd, buf, sizeof buf);
	cout << "response:" << buf << endl;
	//关闭文件描述符
	close(fld);
	//删除session
	dSessionL(path);
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
	//发送upld, 接收success
	write(fd, buf, strlen(buf));
	read(fd, buf, sizeof buf);
	cout << "response:" << buf << endl;
	//发送offset，收到success
	int ofst = rSessionU(path);
	write(fd, to_string(ofst).c_str(), to_string(ofst).size());
	//sleep(1);
	read(fd, buf, sizeof buf);
	//
	strcpy(buf, "suc\0");
	write(fd, buf, 4);
	write(fd, path.c_str(), strlen((path).c_str()));
	//偏移
	int ot = lseek(fld, ofst, SEEK_SET);
	if(ot == -1){
		cerr << "偏移出错" << endl;
	}	
	read(fd, buf, BUF_SIZE);
	while(1){
		//发送文件
		int bytes = read(fld, buf + 1, BUF_SIZE);
		if(bytes < BUF_SIZE){
			buf[0] = '0';
		}
		else{
			buf[0] = '1';
		}
		write(fd, buf, bytes + 1);
		ofst += bytes;
		wSessionU(path, ofst);
		if(buf[0] == '0'){
			break;
		}
		// cout << buf << endl;
	}
	//sleep(1);
	read(fd, buf, sizeof buf);
	cout << "response:" << buf << endl;
	//关闭文件描述符
	close(fld);
	dSessionU(path);
}
int main(){
	//初始化套接字，连接到服务器
	init();
	string command;
	cout << "input username" << endl;
	cin >> command;
	write(fd, command.c_str(), command.size());
	//sleep(1);
	getchar();
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
		else if(op == "del "){
			senddel(command);
		}
		else{
			cout << "invalid command" << endl;
		}
		cout << endl;
	}
	//关闭套接字文件描述符
	close(fd);
	return 0;
}
