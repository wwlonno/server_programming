/* client通过主机名和服务名来访问目标服务器的daytime服务 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	assert(argc == 2);
	// get host infomation
	char *host = argv[1];
	struct hostent *hostinfo = gethostbyname(host);
	assert(hostinfo);
	// get service information
	struct servent *servinfo = getservbyname("daytime", "tcp");
	assert(servinfo);
	printf("daytime port is %d\n", ntohs(servinfo->s_port));

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = servinfo->s_port;
	server_address.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;

	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(sockfd >= 0);

	int ret = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
	printf("errno is %d\n", errno);
	printf("%s\n", strerror(errno));
	assert(ret != -1);

	char buffer[128];
	memset(&buffer, '\0', sizeof(buffer));
	ret = read(sockfd, buffer, 128);
	assert(ret > 0);
	printf("the day time is : %s\n", buffer);
	
	close(sockfd);
	return 0;
}
