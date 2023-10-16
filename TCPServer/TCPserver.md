## TCPserver

+ 头文件不全
+ 查询函数还没写完（不会写）
+ 没有连接客户端验证
+ open的参数不确定

#### Client

+ 全局变量fd表示的是客户端的套接字，在sendlist中调用write、read函数应该使用服务端的套接字？

+ 解析文件路径`string path = command.substr(5, (int)command.size() - 5);`

  + 不是很懂5是什么

  

  

