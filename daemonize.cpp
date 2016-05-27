/* 将一个进程以守护进程的方式运行 */

#include <unistd.h>
#include <stdlib.h>

bool daemonize()
{
	pid_t pid = fork();
	if (pid < 0)
		return false;
	else if (pid > 0)   // parent process
		eixt(0);

	// create file mask
	umask(0);
	//create new session
	pid_t sid = setsid();
	if (sid < 0)
		return false;

	//change the working directory
	if (chdir("/") < 0)
		return false;

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	open("dev/null", O_RDONLY);
	open("dev/null", O_WRONLY);
	open("dev/null", O_RDWR);

	return true;
}
