#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printf("usage : %s ip_address port_number buffer_size\n", basename(argv[0]));
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

	int recvbuf = atoi(argv[3]);
	int len = sizeof(recvbuf);
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf, sizeof(recvbuf));
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf, (socklen_t *)&len);
	printf("the tcp receive buffer size after setting is %d\n", recvbuf);

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
		char buffer[BUF_SIZE];
		memset(buffer, '\0', BUF_SIZE);
		while (recv(connfd, buffer, BUF_SIZE, 0) > 0) { }
		close(connfd);
	}

	close(sock);
	return 0;
}
