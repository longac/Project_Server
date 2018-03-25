#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

//创建套接字
int /*成功返回指向新创建的socket的文件描述符, 失败返回-1*/
Socket(
		int domain, /*使用的哪种协议宏，一般常用AF_INET*/
		int type, /*协议类型 TCP:SOCK_STREAM UDP:SOCK_DGRAM*/
		int protocol /*0代表默认协议*/
		)
{
	int fd = 0;
	fd = socket(domain, type, protocol);
	if(-1 == fd) {
		perror("socket error: ");
		exit(-1);
	}
	return fd;
}

//将创建的套接字绑定到本机的指定IP和PORT
int /*成功返回0，失败返回-1*/
Bind(
		int sockfd, /*要绑定的socket文件描述符*/
		const struct sockaddr *addr, /*地址结构体，包含IP地址和PORT端口号等信息，sockaddr这种结构体被sockadd_in替代*/
		socklen_t addrlen /*地址结构体所占空间*/
		)
{
	int ret = 0;
	ret = bind(sockfd, addr, addrlen);
	if(-1 == ret) {
		perror("bind error: ");
		exit(-1);
	}

	return ret;
}

//设置监听上线数
int /*成功返回0，失败返回-1*/
Listen(
		int sockfd, /*监听的套接字文件描述符*/
		int backlog /*客户端最大连接数*/
		)
{
	int ret = 0;
	ret = listen(sockfd, backlog);
	if(-1 == ret) {
		perror("listen error: ");
		exit(-1);
	}

	return ret;
}

//服务器端接受客户端连接，阻塞等待
int /*成功创建一个新的文件描述符用于与客户端通信，失败返回-1*/
Accept(
		int sockfd, /*要绑定的socket文件描述符*/
		struct sockaddr *addr, /*地址结构体，包含IP地址和PORT端口号等信息，sockaddr这种结构体被sockadd_in替代*/
		socklen_t *addrlen /*地址结构体所占空间*/
		)
{
	int fd;

again:
	fd = accept(sockfd, addr, addrlen);
	if(fd < 0) {
		if((EINTR == errno) || (ECONNABORTED == errno)) {
			goto again;
		}
		else {
			perror("accept error: ");
			exit(-1);
		}
	}

	return fd;
}

//客户端用于连接服务器，阻塞等待
int /*成功返回0，失败返回-1*/
Connect(
		int sockfd, /*要绑定的socket文件描述符*/
		const struct sockaddr *addr, /*地址结构体，包含IP地址和PORT端口号等信息，sockaddr这种结构体被sockadd_in替代*/
		socklen_t addrlen /*地址结构体所占空间*/
		)
{
	int ret = 0;
	ret = connect(sockfd, addr, addrlen);
	if(-1 == ret) {
		perror("connect error: ");
		exit(-1);
	}

	return ret;
}

//封装close函数
int /*成功返回0，失败返回-1*/
Close(
		int fd /*要关闭的文件描述符*/
		)
{
	int ret = 0;

	ret = close(fd);
	if(-1 == ret) {
		perror("close error: ");
		exit(-1);
	}

	return ret;
}
//封装read函数
ssize_t /*成功返回读取的字节数，0代表读到文件末尾，失败返回-1*/
Read(
		int fd,
		void *buf,
		size_t count
		)
{
	ssize_t ret = 0;

again:
	ret = read(fd, buf, count);
	if(-1 == ret) {
		if(EINTR == errno) {
			goto again;
		}
		else {
			return -1;
		}
	}

	return ret;
}

//封装write函数
ssize_t /*成功返回写入的字节数，0代表没有写入数据，失败返回-1*/
Write(
		int fd,
		const void *buf,
		size_t count
		)
{
	ssize_t ret = 0;

again:
	ret = write(fd, buf, count);
	if(-1 == ret) {
		if(EINTR == errno) {
			goto again;
		}
		else {
			return -1;
		}
	}

	return ret;
}

//读取n个字节
ssize_t /*读取字节数*/
Readn(
		int fd, /*文件描述符*/
		void *buf, /*缓冲区*/
		size_t n /*要读取的长度*/
		)
{
	ssize_t nread = 0;
	size_t nleft = 0;
	char *ptr;

	ptr = buf;
	nleft = n;

	while(nleft > 0) {
		nread = read(fd, ptr, nleft);
		if(nread < 0) {
			if(EINTR == errno) {
				nread = 0;
			}
			else {
				return -1;
			}
		}
		else if(0 == nread){
			break;
		}

		nleft -= nread;
		ptr += nread;
	}

	return n - nleft;
}

//写入n个字节
ssize_t /*写入字节数*/
Writen(
		int fd, /*文件描述符*/
		const void *buf, /*缓冲区*/
		size_t n /*缓冲区大小*/
		)
{
	ssize_t nwrited = 0;
	size_t nleft = 0;
	const char *ptr;

	ptr = buf;
	nleft = n;

	while(nleft > 0) {
		nwrited = write(fd, ptr, nleft);
		if(nwrited < 0) {
			if(EINTR == errno) {
				nwrited = 0;
			}
			else {
				return -1;
			}
		}
		else if(0 == nwrited){
			break;
		}
		ptr += nwrited;
		nleft -= nwrited;
	}

	return n - nleft;
}

//缓冲读取字节
static ssize_t /*1代表读取的字节数 0代表文件已经读完*/
ReadBuff(
		int fd, /*文件描述符*/
		char *ptr /*当前读取的字符*/
		)
{
	static int nRead; /*缓冲区中读取剩下的字节大小*/
	static char *ptr_read; /*缓冲区读取位置*/
	static char read_buf[128]; /*缓冲区大小,数据读取上限*/

	/*先检查缓冲区中的数据有没有读完*/
	if(nRead <= 0) {
again:
		//读取指定大小到缓冲区中
		nRead = read(fd, read_buf, sizeof(read_buf));
		if(nRead < 0) {
			if(errno == EINTR) {
				goto again;
			}
			return -1;
		}
		else if(nRead == 0) {
			return 0;
		}

		//更新读取指针
		ptr_read = read_buf;
	}

	/*没有读完，从缓冲区读取一个字节*/
	nRead--;
	*ptr = *ptr_read++;

	return 1;
}

//读取一行
ssize_t /*大于0表示读取到一行的字节,-1表示读取失败*/
ReadLine(
		int fd,
		void *buf,
		size_t n
		)
{
	ssize_t nRead = 0;
	ssize_t ret = 0;
	char ch;
	char *ptr;

	ptr = buf;
	for(nRead = 1; nRead < n; nRead++) {
		//从缓冲区中读取一个字节
		ret = ReadBuff(fd, &ch);
		if(1 == ret) {
			*ptr++ = ch;
			//督导\n结束读取
			if('\n' == ch) {
				break;
			}
		}
		else if(0 == ret) {
			*ptr = 0;
			return nRead - 1;
		}
		else {
			return -1;
		}
	}

	*ptr = 0;
	return nRead;
}
