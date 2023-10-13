#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
using namespace std;
int main(){
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){
		cerr << "Create socket fail" << endl;
		exit(1);
	}
	else{
		cout << "Create socket success" << endl;
	}
	sockaddr_in sd;
	inet_aton("0.0.0.1", &(sd.sin_addr));
	cout << sd.sin_addr.s_addr << endl;
	sd.sin_family = AF_INET;
	sd.sin_port = 6789;
	sd.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(fd, (sockaddr *)&sd, sizeof  sd) == -1){
		cerr << "Bind port fail" << endl;
		exit(1);
	}
	else{
		std::cout << "Bind port success" << std::endl;
	}
	while(1);
	return 0;
}
