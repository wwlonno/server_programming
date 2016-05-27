/* 使用splice函数零拷贝的回射服务器， 它将客户端发送的数据原样返回给客户端 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printf("usage : %s ip_address port_number\n", basename(argv[0]));
		return 1;
	}

	const char *ip = argv[1];
	int port = atoi(argv[2]);
	
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ip, &address.sin_addr);

	int sock = socket(PF_INET, SOCK_STREAM, 0);
	assert(sock >= 0);

	int ret = bind(sock, (struct sockaddr *)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(sock, 5);
	assert(ret != -1);

	struct sockaddr_in client_address;
	socklen_t client_address_len = sizeof(client_address);
	int connfd = accept(sock, (struct sockaddr *)&client_address, &client_address_len);
	if (connfd < 0)
		printf("errno is %d\n", errno);
	else
	{
		int pipefd[2];
		ret = pipe(pipefd);
		assert(ret != -1);
		/* 数据从connfd移到pipefd[1] */
		ret = splice(connfd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
		assert(ret != -1);
		/* 数据从pipefd[0]移到connfd */
		ret = splice(pipefd[0], NULL, connfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
		assert(ret != -1);

		close(connfd);
	}

	close(sock);
	return 0;
}
