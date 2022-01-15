#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>
#include "../../libs/serverlib/server.h"

static void skeleton_daemon()
{
	pid_t process_id = 0;
	pid_t sid = 0;
	
	// Create child process
	process_id = fork();

	// Indication of fork() failure
	if (process_id < 0)
	{
		printf("Fork failed!\n");
		
		// Return failure in exit status
		exit(1);
	}

	// PARENT PROCESS. Need to kill it.
	if (process_id > 0)
	{
		printf("process_id of child process %d \n", process_id);
		
		// return success in exit status
		exit(0);
	}

	//unmask the file mode
	umask(0);

	//set new session
	sid = setsid();

	if(sid < 0)
	{
		// Return failure
		exit(1);
	}

	// Change the current working directory to root.
	chdir("/");
}

int main()
{
	skeleton_daemon();

	while (1)

	{

		sleep(1);

		telem_init();

		server();

	}

	return EXIT_SUCCESS;
}

