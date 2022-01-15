#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define portNum 3005
#define ip "127.0.0.1"
#define bufSize 1024

int initClient(int *fd)
{
	*fd = socket(AF_INET, SOCK_STREAM, 0);
	if (*fd < 0)
	{
		perror("Error at creating the socket\n");
		exit(0);
	}

	struct sockaddr_in client_addr;

	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(portNum);

	if (inet_pton(AF_INET, ip, &client_addr.sin_addr) <= 0)
	{
		perror("Error at converting the address\n");
		return errno;
	}

	if (connect(*fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
	{
		perror("Error at connecting to server.\n");
		return errno;
	}

	// Set socket as nonblocking
	// So when there's no message to read
	// The socket won't be stuck there
	int status = fcntl(*fd, F_SETFL, fcntl(*fd, F_GETFL, 0) | O_NONBLOCK);

	if (status == -1)
	{
		perror("Error calling fcntl\n");
		return errno;
	}

	return 0;
}

void printBuffer(char *buffer)
{
	// Decrypts the message format recieved from server
	int nr = 1;
	char *token;
	char sep[2] = "|";
	token = strtok(buffer, sep);

	while (token != NULL)
	{
		if (nr % 2 == 1)
		{
			printf("User %s: ", token);
		}
		else
		{
			printf("%s", token);
		}
		nr++;
		token = strtok(NULL, sep);
	}
}

void printLastMessage(int *fd)
{
	char buffer[bufSize + 1] = {0};
	int valRead;
	valRead = recv(*fd, buffer, bufSize, 0);
	
	// Because the socket has the NONBLOCK flag 
	// there's the possibility that the client will try to recieve
	// a message before the server actually sent one
	// so this while is to ensure the server actually sends 
	// the last message
	while (valRead == -1)
		valRead = recv(*fd, buffer, bufSize, 0);
	
	if (valRead == -1)
	{
		puts(strerror(errno));
		exit(1);
	}
	
	
	// Decrypting the message from the server
	buffer[valRead] = '\0';
	int nr = 1;
	char *token;
	char sep[2] = "|";
	token = strtok(buffer, sep);
	while (token != NULL)
	{
		if (nr == 1) // first message in buffer is "Connected..." so just print
		{
			printf("%s\n", token);
		}
		else
		{
			if (nr == 3 && strcmp(token, "nmsgf") == 0) // display no message found
				printf("Be the first one to write in this channel!\n");
			else if (nr == 3 && strcmp(token, "nmsgf") != 0) // display message from user
				printf("%s\n", token);
			if (nr == 2 && strcmp(token, "100") != 0) // display user
				printf("User: ");
		}
		nr++;
		token = strtok(NULL, sep);
	}
	
}

int testConnection(int *fd)
{
	char buffer[bufSize + 1] = {0};
	printf("Introduce path: ");
	fgets(buffer, bufSize, stdin);
	buffer[strlen(buffer)] = '\0';

	// Send the path to the server to connect
	send(*fd, buffer, strlen(buffer), MSG_NOSIGNAL); // send what channel path the user wants to connect
	
	printLastMessage(fd);

	return 0;
}

void readRequest(int *fd)
{
	int dataAvailable;
	ioctl(*fd, FIONREAD, &dataAvailable);
        
	if (!dataAvailable) // If there's no data to read 
		return;
	
	char *tmp =(char*) malloc(bufSize + 1);
	char *msg = (char*) malloc(bufSize + 1);
	int valRead = recv(*fd, msg, bufSize, MSG_NOSIGNAL);
	if (valRead > 0)
	{
		msg[valRead] = '\0';
		strcpy(tmp, msg);
		printBuffer(tmp); // Go decrypt server message
	}
	
	free(tmp);
	free(msg);
}

int writeRequest(int *fd)
{
	char buffer[bufSize + 1] = {0};
	printf("Client: ");
	fgets(buffer, bufSize, stdin);

	int lg = strlen(buffer);
	if (*buffer && buffer[lg] == '\n')
		buffer[lg] = '\0';
	if (strcmp(buffer, "#\n") == 0)
	{
		send(*fd, "0xEND", bufSize, 0);
		return 1;
	}
	else 
		send(*fd, buffer, bufSize, 0);
	return 0;
}

void sublisher()
{
	int client;
	if (initClient(&client) != 0)
	{
		exit(0);
	}

	if (testConnection(&client) != 0)
		exit(0);


	int isRunning = 1;

	while (isRunning)
	{
		readRequest(&client);
		if (writeRequest(&client) == 1)
			isRunning = 0;
	}

	puts("Terminated\n");
	close(client);
}

void publisher()
{
	int client;
	if (initClient(&client) != 0)
	{
		exit(0);
	}

	if (testConnection(&client) != 0)
		exit(0);


	int isRunning = 1;

	while (isRunning)
	{
		if (writeRequest(&client) == 1)
			isRunning = 0;
	}

	puts("Terminated\n");
	close(client);
}

void subscriber()
{
	int client;
	if (initClient(&client) != 0)
	{
		exit(0);
	}

	if (testConnection(&client) != 0)
		exit(0);


	int isRunning = 1;

	while (isRunning)
	{
		readRequest(&client);
	}

	puts("Terminated\n");
	close(client);
}
