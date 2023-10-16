**云存储**

**基础功能**

上传，下载，查询，进阶：断点续传

**知识点**

Linux基础指令，C语言，

目录IO，opendir,readdir

文件IO

Open

Read

Writ

Lseek

Close

Linux TCP网络编程：



**TCP报文协议**

**请求目录**

**list /home**

**下载**

**load /home/1.txt**

**上传**

**upld  /1.txt**

**传输**

**file 文件流**

| 类型     | 功能                   | 格式                            | 示例                                        |
| -------- | ---------------------- | ------------------------------- | ------------------------------------------- |
| list     | 请求指定的文件目录信息 | list [pathname]                 | list /home                                  |
| download | 下载某一文件           | load [pathname/filename]        | load /home/a.txt                            |
| upload   | 上传某一文件           | upld [pathname/filename] [file] | upld /home/a.txt 53241521412512412412512412 |

例子

**1**

客户端：list /home

服务端：A.txt B.txt C.txt

**2**

客户端：load /home/a.txt

服务端：这里是a.txt的内容

**3**

客户端：upload /home/a.txt 这里是a.txt的内容

此时服务端不需要返回信息。
