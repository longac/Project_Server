#ifndef _WRAP_H_
#define _WRAP_H_

#include <sys/socket.h>

//封装socket函数
int Socket(int domain, int type, int protocol);
//封装bind函数
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
//封装listen函数
int Listen(int sockfd, int backlog);
//封装accept函数
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
//封装connect
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
//封装read函数
ssize_t Read(int fd, void *buf, size_t count);
//封装write函数
ssize_t Write(int fd, const void *buf, size_t count);
//封装close函数
int Close(int fd);
//读取n个字节
ssize_t Readn(int fd, void *buf, size_t n);
//写入n个字节
ssize_t Writen(int fd, const void *buf, size_t n);
//缓冲读取
ssize_t ReadBuff(int fd, char *ptr);
//读取一行
ssize_t ReadLine(int fd, void *buf, size_t n);

#endif
