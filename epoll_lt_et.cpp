#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd(int epollfd, int fd, bool enable_et)
{
	epoll_eventevent;
	event.data.fd = fd;
	event.events = EPOLLIN;
	if (enable_et)
		event.events |= EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

void lt(epoll_event *events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	for (int i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;
		if (sockfd == listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_address_len = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_address_len);
			addfd(epollfd, connfd, false);
		}
		else if (events[i].events & EPOLLIN)
		{
			// 因为这里使用的是lt模式，所以这段代码会重复触发
			printf("event trigger once\n");
			memset(buf, '\0', BUFFER_SIZE);
			int ret = recv(sockfd, buf, BUFFER_SIZE-1, );
			if (ret <= 0)
			{
				close(sockfd);
				continue;
			}
			printf("get %d bytes of content: %s\n", ret, buf);
		}
		else
			printf("something else happend\n");
	}
}

void et(epoll_event *events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	for (int i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;
		if (sockfd == listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_address_len = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_address_len);
			addfd(epollfd, connfd, true);
		}
		else if (events[i].events & EPOLLIN)
		{
			// 因为这里使用的是et模式，所以这段代码只会运行一次，所以要循环把socket能读的所有数据读出来
			printf("event trigger once\n");
			while(1)
			{
				memset(buf, '\0', BUFFER_SIZE);
				int ret = recv(sockfd, buf, BUFFER_SIZE-1, 0);
				if (ret < 0)
				{
					// 对于非阻塞IO，下面的条件成立表示数据已全部读取完毕
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
					{
						printf("read later\n");
						break;
					}
					close(sockfd);
					break;
				}
				else if (ret == 0)
					close(sockfd);
				else
					printf("get %d bytes of conten: %s\n", ret, buf);
			}
		}
		else
			printf("somthing else happened\n");
	}
}

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printf("usage: %s ip_address port_number\n", basename(argv[0]));
		return 1;
	}

	const char *ip = argv[1];
	int port = atoi(argv[2]);

	int ret = 0;
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ip, &address.sin_addr);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(listenfd, 5);
	assert(ret != -1);

	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);    //这个epollfd同时处理listenfd和connfd
	assert(epollfd != -1);
	addfd(epollfd, listenfd, true);

	while(1)
	{
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if (ret < 0)
		{
			printf("epoll failure\n");
			break;
		}

		lt(events, ret, epollfd, listenfd);
		//et(events, ret, epollfd, listenfd);
	}

	close(listenfd);
	return 0;
}
