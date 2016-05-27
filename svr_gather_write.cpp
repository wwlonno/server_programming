/* 使用writev函数将分成两部分的http应答发送出去， 一部分内存存了1个状态行，多个头部字段和1个空行， 另一部分内存存了文件的内容
 * 需要使用writev来把两个不同内存里的内容都发送出去，这称为集中写，其对应的是分散读，就是将读到的内容分散存到不同的内容中去。
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>  //用来获取文件状态

#define BUFFER_SIZE 1024

static const char *status_line[2] = {"200 OK", "500 Internal server error"};

int main(int argc, char *argv[])
{
	if (argc <= 3)
	{
		printf("usage : %s ip_address port_number filename\n", basename(argv[0]));
		return 1;
	}

	const char *ip = argv[1];
	int port = atoi(argv[2]);
	const char *file_name = argv[3];

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
		char header_buf[BUFFER_SIZE];
		memset(header_buf, '\0', BUFFER_SIZE);

		char *file_buf;
		struct stat file_stat;
		bool valid = true;
		
		int len = 0;
		if (stat(file_name, &file_stat) < 0)  //目标文件不存在
			valid = false;
		else
		{
			if (S_ISDIR(file_stat.st_mode))
				valid = false;
			else if (file_stat.st_mode & S_IROTH)  //其他用户读权限
			{
				int fd = open(file_name, O_RDONLY);
				file_buf = new char[file_stat.st_size + 1];
				memset(file_buf, '\0', file_stat.st_size+1);
				if (read(fd, file_buf, file_stat.st_size) < 0)
					valid = false;
			}
			else
				valid = false;
		}

		if (valid)
		{
			// construct header_buf
			// header_buf is a string of http response
			ret = snprintf(header_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[0]);
			len += ret;
			ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "Content-Length: %d\r\n", file_stat.st_size);
			len += ret;
			ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");

			struct iovec iv[2];
			iv[0].iov_base = header_buf;
			iv[0].iov_len = strlen(header_buf);
			iv[1].iov_base = file_buf;
			iv[1].iov_len = file_stat.st_size;
			ret = writev(connfd, iv, 2);
		}
		else  // 目标文件无效， 就通知客户端， 服务器发生了内部错误
		{
			ret = snprintf(header_buf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[1]);
			len += ret;
			ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");
			send(connfd, header_buf, strlen(header_buf), 0);
		}

		close(connfd);
		delete []file_buf;
	}

	close(sock);
	return 0;
}

