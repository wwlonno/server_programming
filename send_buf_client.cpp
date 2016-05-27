#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 512

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printf("usage : %s ip_address port_number buffer_size\n", basename(argv[0]));
		return 1;
	}

	const char *ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	inet_pton(AF_INET, ip, &server_address.sin_addr);

	int sock = socket(PF_INET, SOCK_STREAM, 0);
	assert(sock >= 0);

	int sendbuf = atoi(argv[3]);
	int len = sizeof(sendbuf);
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf, sizeof(sendbuf));
	getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf, (socklen_t *)&len);
	printf("the tcp send buffer size after setting is %d\n", sendbuf);

	if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) != -1)
	{
		char buffer[BUF_SIZE];
		memset(buffer, 'a', BUF_SIZE);
		send(sock, buffer, BUF_SIZE, 0);
	}

	close(sock);
	return 0;
}

