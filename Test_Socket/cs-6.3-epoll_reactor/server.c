/*
 * 高并发服务器 epoll-反应堆模式
 * stanley
 * 2018-04-16
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include "wrap.h"

#define BUFLEN 4096
#define SERV_PORT 8008	//服务器端口号
#define MAX_EVENTS 1024	//最大监听上线

//自定义结构体
struct event_sty {
	int fd;												//要监听的文件描述符
	int events;											//要监听的事件
	void* arg;											//泛型参数
	void (*call_back)(int fd, int event, void* arg);	//回调函数
	int status;											//是否在红黑树上 1->在红黑树上  0->不在红黑树上
	char buf[BUFLEN];									//
	int len;											//
	long last_active;									//记录每次加入红黑树 g_efd的时间
};

/*全局变量区*/
int g_efd;	//全局变量，用来保存epoll_create,返回的文件描述符
struct event_sty g_events[MAX_EVENTS + 1];	//全局变量，用来保存满足事件的文件描述符对应的结构体数组

//监听端口初始化
void socketInit(int, short);

//初始化上树结构体成员信息
void eventSet(struct event_sty*, int, void (*)(int, int, void*), void*);
//有监听事件的文件描述符及结构体上树
void eventAdd(struct event_sty*, int, int);
//将该结点从红黑书上摘下
void eventDel(struct event_sty*, int);

//回调函数,用于建立服务器客户端的连接
void accpetConnect(int, int, void*);

//回调函数,用于读取客户端发来的数据
void recvData(int, int, void*);

//回调函数,用于回射客户端发来的消息
void sendData(int, int, void*);

int
main(int argc, const char* argv[])
{
	unsigned short port = SERV_PORT;

	//用户指定端口 否则使用默认端口
	if(2 == argc) {
		port = atoi(argv[1]);
	}
	
	//创建红黑树结点
	g_efd = epoll_create(MAX_EVENTS + 1);	//最大监听加上lfd
	if(g_efd < 0) {
		printf("epoll_create in %s err %s\n", __func__, strerror(errno));
	}

	//初始化端口信息
	socketInit(g_efd, port);

	struct epoll_event events[MAX_EVENTS + 1];	//本地存放满足事件的文件描述符数组

	int i = 0;

	printf("server running at port : %d\n", port);
	while(1) {
		/*超时验证*/

		/*监听红黑树的结点，将满足事件的文件描述符添加到events数组中, 阻塞时间为1秒，没有事件，返回0*/
		int nReady = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);
		if(nReady < 0) {
			printf("epoll_wait in %s err %s\n", __func__, strerror(errno));
			break;
		}

		for(i = 0; i < nReady; ++i) {
			//接收泛型指针
			struct event_sty *eventTemp = (struct event_sty*)events[i].data.ptr;
			//读事件
			if((events[i].events & EPOLLIN) && (eventTemp->events & EPOLLIN)) {
				eventTemp->call_back(eventTemp->fd, eventTemp->events, eventTemp->arg);
			}
			//写事件
			if((events[i].events & EPOLLOUT) && (eventTemp->events & EPOLLOUT)) {
				eventTemp->call_back(eventTemp->fd, eventTemp->events, eventTemp->arg);
			}
		}
	}
	

	return 0;
}

//监听端口初始化
void
socketInit(
		int efd,	//epoll红黑树文件描述符
		short port	//服务器端口号
		)
{
	int lfd = Socket(AF_INET, SOCK_STREAM, 0);

	//设置端口复用
	int opt;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//设置非阻塞监听
	int flag = fcntl(lfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(lfd, F_SETFL, flag);

	//设置服务端socket信息
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	
	//绑定端口信息
	Bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	//设置监听上线
	Listen(lfd, 128);

	//设置监听事件属性
	//使用最后一个作为监听事件的文件描述符
	//回调accpetConnect函数
	//将本身作为参数传给回调函数
	eventSet(&g_events[MAX_EVENTS], lfd, accpetConnect, &g_events[MAX_EVENTS]);

	//监听事件上树
	eventAdd(&g_events[MAX_EVENTS], efd, EPOLLIN);
	
	return;
}


//初始化上树结构体成员信息
void
eventSet(
		struct event_sty* event,			//自定义结构体
		int fd,								//要初始化的文件描述符
		void (*call_back)(int, int, void*),	//回调函数
		void* arg							//回调函数参数
		)
{
	event->fd = fd;
	event->call_back = call_back;
	event->events = 0;
	event->arg = arg;
	event->status = 0;
	//memset(event->buf, 0, sizeof(event->buf));
	//event->len = 0;
	event->last_active = time(NULL);

	return ;
}


//有监听事件的文件描述符及结构体上树
void
eventAdd(
		struct event_sty* event,	//要上树的结构体信息
		int efd,					//epoll红黑树树根
		int events					//上树事件
		)
{
	struct epoll_event ep_event = {0, {0}};
	int op;

	//设置回调函数
	ep_event.data.ptr = event;
	//设置监听事件类型
	ep_event.events = event->events = events;

	if(1 == event->status) {	//已在红黑树上
		op = EPOLL_CTL_MOD;		//修改其属性
	} else {					//不在红黑树上
		op = EPOLL_CTL_ADD;		//将其加入到红黑树上
		event->status = 1;		//修改其上树的状态
	}

	if(epoll_ctl(efd, op, event->fd, &ep_event) < 0) {
		printf("event add failed [fd = %d], events[%d]\n", event->fd, events);
	} else {
		printf("event add OK [fd = %d], op = %d, events[%d]\n", event->fd, op, events);
	}

	return ;
}

//将该结点从红黑书上摘下
void
eventDel(
		struct event_sty* event,
		int efd)
{
	struct epoll_event ep_event = {0, {0}};

	if(1 != event->status) {
		return ;
	}

	ep_event.data.ptr = event;
	event->status = 0;
	//将结点从红黑树上摘下
	epoll_ctl(efd, EPOLL_CTL_DEL, event->fd, &ep_event);
	
	return ;
}

//回调函数	文件描述符就绪
void
accpetConnect(
		int lfd,		//监听文件描述符
		int events,		//事件
		void* arg		//参数
		)
{
	struct sockaddr_in clie_addr;
	socklen_t len = sizeof(clie_addr);
	int cfd;
	int i;

	if(-1 == (cfd = Accept(lfd, (struct sockaddr*)&(clie_addr), &len))) {
		printf("%s: accpet %s\n", __func__, strerror(errno));
		return ;
	}

	do {
		//找到第一个没有在树上的结点
		for(i = 0; i < MAX_EVENTS; ++i) {
			if(0 == g_events[i].status) {
				break;
			}
		}

		if(MAX_EVENTS == i) {
			printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
			break;
		}

		int flag = 0;
		if((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) {
			printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
			break;
		}

		//设置新的结构体属性，挂在树上
		eventSet(&g_events[i], cfd, recvData, &g_events[i]);
		eventAdd(&g_events[i], g_efd, EPOLLIN);		//将cfd设置成监听读事件，添加到树上
	} while(0);

	printf("new connect [%s:%d][time:%ld], pos[%d]\n",
			inet_ntoa(clie_addr.sin_addr),
			ntohs(clie_addr.sin_port),
			g_events[i].last_active,
			i);

	return ;
}


//回调函数,用于读取客户端发来的数据
void
recvData(
		int cfd,		//建立连接后的客户端文件描述符
		int events,		//事件
		void* arg)		//回掉函数
{
	struct event_sty* event = (struct event_sty*) arg;
	int nRead;

	nRead = recv(cfd, event->buf, sizeof(event->buf), 0);
	
	//将该节点从红黑树上摘除
	eventDel(event, g_efd);

	if(nRead > 0) {
		event->len = nRead;
		event->buf[nRead] = '\0';
		printf("recv[%d]: %s\n", cfd, event->buf);

		eventSet(event, cfd, sendData, event);
		eventAdd(event, g_efd, EPOLLOUT);
	} else if(0 == nRead) {
		close(event->fd);
		printf("[fd = %d] [pos = %ld], closed\n", cfd, event - g_events);
	} else {
		close(event->fd);
		printf("[fd = %d], error[%d] : %s\n", cfd, errno, strerror(errno));
	}

	return ;
}

//回调函数,用于回射客户端发来的消息
void
sendData(
		int cfd,
		int events,
		void* arg
		)
{
	struct event_sty* event = (struct event_sty*)arg;
	int nSend;

	//直接将接收到的内容回发给客户端
	nSend = send(cfd, event->buf, event->len, 0);

	if(nSend > 0) {
		printf("send[fd = %d], [%d] %s\n", cfd, nSend, event->buf);
		eventDel(event, g_efd);

		eventSet(event, cfd, recvData, event);
		eventAdd(event, g_efd, EPOLLIN);
	} else {
		close(event->fd);
		eventDel(event, g_efd);
		printf("send[fd = %d], error : %s\n", cfd, strerror(errno));
	}

	return ;
}
