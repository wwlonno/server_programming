/* 利用sendfile函数将文件从服务端传到客户端 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

int main(int argc, char *argv[])
{
	if (argc <= 3)
	{
		printf("usage : %s ip_address port_number filename\n", argv[0]);
		return 1;
	}

	const char *ip = argv[1];
	int port = atoi(argv[2]);
	const char *file_name = argv[3];

	int filefd = open(file_name, O_RDONLY);
	assert(filefd > 0);
	struct stat stat_buf;
	fstat(filefd, &stat_buf);

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
	int connfd = accept(sock, (struct sockaddr *)&address, &client_address_len);	
	if (connfd < 0)
		printf("errno is %d\n", errno);
	else
	{
		sendfile(sock, filefd, NULL, stat_buf.st_size);
		close(connfd);
	}

	close(sock);
	return 0;
}
