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

#define MAX_USERS 30
#define portNum 3005
#define MAX_QUEUE_USERS 5
#define bufSize 1024

#define MAX_CHANNELS 100
#define MAX_NAME_LENGTH 30
#define MAX_MSG_LENGTH 1024
#define MAX_USERS_CHANNEL 5

typedef struct telem_t
{
    char *name;             // channel name
    char *msg;              // last message sent on this channel
    int noChdr;             // number of children
    struct telem_t **child; // vector of children

} telem_t;

/// Struct for memorizing client connection
typedef struct conn_t
{
    int sd;           // socket descriptor for client connection
    telem_t *channel; // channel for connection
} conn_t;

telem_t *root = NULL; // root


telem_t *telem_create(const char *givenName)	/// creating a node
{
    telem_t *newNode;
    newNode = (telem_t *)malloc(sizeof(telem_t));

    /// set channel name
    newNode->name = (char *)malloc(MAX_NAME_LENGTH * sizeof(char));
    strcpy(newNode->name, givenName);

    /// set last msg
    newNode->msg = (char *)malloc(MAX_MSG_LENGTH * sizeof(char));
    strcpy(newNode->msg, "|100|nmsgf"); // code for no message on this channel yet
    // "sent" by a sd = 100 

    /// list of children
    newNode->noChdr = 0;
    newNode->child = (telem_t **)malloc(MAX_CHANNELS * sizeof(telem_t *));

    return newNode;
}

void add_child(telem_t *parent, telem_t *child) /// adding a child to a parent
{
    if (parent->noChdr == MAX_CHANNELS)
    {
        perror("This channel is already full");
        return;
    }
    strcpy(child->msg, parent->msg);
    parent->child[parent->noChdr] = child;
    parent->noChdr++;
}

void msgPropagation(telem_t *start, const char *msg) /// copy last message to childs
{
	// DFS where we copy the message in each node
	
    if (start == NULL)
    {
        perror("start is null");
        exit(1);
    }
    strcpy(start->msg, msg);

    for (int i = 0; i < start->noChdr; ++i)
    {
        msgPropagation(start->child[i], msg);
    }
}

telem_t *search(telem_t *start, telem_t *givenNode)
{
    for (int i = 0; i < start->noChdr; ++i)
    {
        if (givenNode == start->child[i])
        {
            return givenNode;
        }
        telem_t *next = search(start->child[i], givenNode);
        if (next)
            return next;
    }
    return NULL;
}

telem_t *searchParent(telem_t *start, telem_t *givenNode)
{
    if (start == givenNode)
    {
        return NULL;
    }
    for (int i = 0; i < start->noChdr; ++i)
    {
        if (givenNode == start->child[i])
        {
            return start;
        }
        if (start->child[i] == NULL)
        {
            continue;
        }
        telem_t *next = searchParent(start->child[i], givenNode);
        if (next)
            return next;
    }
    return NULL;
}

int isIndirectChild(telem_t *looking_for, telem_t *current)
{
    // Given a parent and a node return 1 if the given node is an indirect child of parent
    // or 0 if it isn't
    // (recursevely)
    if (current == NULL)
		return 0;
    if (current == looking_for)
        return 1;
    int found = 0;
    for (int i = 0; i < current->noChdr; ++i)
    {
        if (current->child[i] != NULL)
        {
			printf("is child: %s\n", current->child[i]->name);
            found = isIndirectChild(looking_for, current->child[i]);
		}
		if (found == 1)
			break;
    }

    return found;
}

void swap(telem_t **swapX, telem_t **swapY)
{
    telem_t **aux = NULL;
    *aux = *swapX;
    *swapX = *swapY;
    *swapY = *aux;
}

void telem_destroy(telem_t **node) /// delete subtree starting with node
{
    if (*node == NULL)
    {
        return;
    }

    telem_t *cNode = *node;

    for (int i = 0; i < cNode->noChdr; ++i)
        if (cNode->child[i])
        { /// if there is a child delete it as well
            telem_destroy(cNode->child + i);
        }

    telem_t *parentNode = searchParent(root, cNode);

    if (parentNode != NULL)
    {

        for (int i = 0; i < parentNode->noChdr; ++i)
        {
            if (parentNode->child[i] == cNode)
            {
                parentNode->child[i] = NULL;
            }
        }
    }
    free(cNode->name);
    free(cNode->msg);
    free(cNode->child);
    free(cNode);
    *node = NULL;
}

void telem_deleteLeaf(telem_t **node) /// delete leaf
{
    if (*node == NULL)
    {
        perror("Node is NULL");
        return;
    }

    telem_t *cNode = *node;

    if (cNode->noChdr)
    {
        perror("The channel contains sub-channels!");
    }
    else
    {
        telem_t *parentNode = searchParent(root, cNode);

        if (parentNode != NULL)
        {

            for (int i = 0; i < parentNode->noChdr; ++i)
            {
                if (parentNode->child[i] == cNode)
                {
                    parentNode->child[i] = NULL;
                }
            }
        }

        free(cNode->name);
        free(cNode->msg);
        free(cNode->child);
        free(cNode);
        *node = NULL;
    }
}

void telem_delete(telem_t *node) /// node deletion
{
    if (node == root)
    {
        telem_destroy(&root);
        return;
    }

    telem_t *parent = searchParent(root, node);

    if (parent->noChdr + node->noChdr >= MAX_CHANNELS)
    {
        perror("It exceeds the number of sub-channels a channel could have!");
        return;
    }
    memcpy(parent->child + (parent->noChdr), node->child, node->noChdr * sizeof(telem_t *));
    parent->noChdr += node->noChdr;
    int k;
    for (k = 0; k < parent->noChdr; k++)
    {
        if (parent->child[k] == node)
        {
            break;
        }
    }
    swap(&parent->child[k], &parent->child[parent->noChdr - 1]);
    parent->noChdr--;
    free(node->name);
    free(node->msg);
    free(node->child);
    free(node);
}

void telem_insertNode(telem_t *child, telem_t *parent)
{
    if (parent->noChdr == MAX_CHANNELS)
    {
        perror("It exceeds the number of sub-channels a channel could have!");
        return;
    }

    parent->child[parent->noChdr] = child;
    parent->noChdr = parent->noChdr + 1;
}

void free_list_of_channels(char **channels)
{
    for (int i = 0; i < 10; i++)
    {
        free(channels[i]);
    }

    free(channels);
}

char **get_channels_from_path(char *path, int *num) /// split a path
{
    char copy[bufSize + 1] = {0}; // For not ruining the path with strtok
    memset(copy, 0, bufSize + 1);
    strncpy(copy, path, strlen(path)- 1); // last char is '\n'
	copy[strlen(path)] = '/';
    char sep[2] = "/"; // separator for channel names
    char *token = strtok(copy, sep);
    char **channels = (char **)malloc(10 * sizeof(char *)); // vector of string
    

    for (int i = 0; i < 10; ++i)
    {
        channels[i] = (char *)malloc(500 * sizeof(char));
		memset(channels[i], 0, 500);
    }

    int channelCounter = 0;
    while (token != NULL)
    {
        strcpy(channels[channelCounter], token); // add channel name
        channelCounter++;
        token = strtok(NULL, sep);
    }
    *num = channelCounter;
    return channels;
}

telem_t *telem_create_from_path(char *path)
{
    int length;
    char **channels;
    channels = get_channels_from_path(path, &length); /// !!!spargerea in canale o facem de fiecare data cand apelam functia asta si functia cu search node
                                                      /// putem optimiza si sa transformam path ul in server da nu e oblig ca path ul e mic relativ

    telem_t *node = root;            /// we start at the root of the tree
    for (int i = 0; i < length; i++) /// iterate through the path
    {
        int found = 0;

        for (int j = 0; j < node->noChdr; j++)
        {
            if (strcmp(channels[i], node->child[j]->name) == 0) /// if we found the next channel in the path
            {
                node = node->child[j]; /// move to that node
                found = 1;
            }
        }

        if (found == 0) /// if we didn t find the channel -> create it
        {
            telem_t *new_channel = telem_create(channels[i]);
            add_child(node, new_channel);
            node = new_channel;
        }
    }
    
    free_list_of_channels(channels);
    
    return node; /// return last made channel -> exactly the channel the user wanted
}

struct sockaddr_in initServer(int *fd)
{
    *fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*fd < 0)
    {
        perror("Error at creating the socket\n");
        exit(0);
    }

    struct sockaddr_in server_addr;
    socklen_t size = sizeof(server_addr);

    // Adding the server addres - localhost:3005
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNum);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*fd, (struct sockaddr *)&server_addr, size) < 0)
    {
        perror("Error at binding the socket\n");
        exit(0);
    }

    return server_addr;
}

int listenServer(int fd)
{
    if (listen(fd, MAX_QUEUE_USERS) < 0)
    {
        perror("Error at listenting\n");
        return errno;
    }

    puts("Server is up and running...");
    return 0;
}

int initSSet(fd_set *sockets, int fd, conn_t fdConn[])
{
    // clear the socket set
    FD_ZERO(sockets);
    // add master socket to set
    FD_SET(fd, sockets);
    int max_sd = fd;

    // add child sockets to set
    for (int i = 0; i < MAX_USERS; i++)
    {
        int sd = fdConn[i].sd;
        if (sd > 0)
            FD_SET(sd, sockets);
        if (sd > max_sd)
            max_sd = sd;
    }

    return max_sd;
}

int addNewSocket(int fdListen, conn_t fdConn[], struct sockaddr_in server_addr, socklen_t size)
{
    int new_socket = accept(fdListen, (struct sockaddr *)&server_addr, &size);
    if (new_socket < 0)
    {
        perror("Error at accepting clients");
        return errno;
    }

    printf("New connection, socket fd is %d, ip is: %s, port: %d \n", new_socket, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    int i;
    
    // add new socket to array of sockets
    for (i = 0; i < MAX_USERS; i++) 	// if position is empty
        if (fdConn[i].sd == 0 && fdConn[i].channel == NULL)
        {
            fdConn[i].sd = new_socket; 	// mark the new socket descriptor
            char buffer[bufSize] = {0};
            recv(new_socket, buffer, bufSize, MSG_NOSIGNAL); 	// recieve the channel
            char path[bufSize + 6] = {0};
        
            strcat(path, buffer);
            
            fdConn[i].channel = telem_create_from_path(path); // get pointer of channel node
            if (fdConn[i].channel == NULL) // One last verification
            {
                perror("channel is null!");
                exit(1);
            }
            printf("Joined channel %s\n", fdConn[i].channel->name);
            break;
        }

    char ok[25] = {0};
    strcpy(ok, "Connected..."); 
    send(new_socket, ok, strlen(ok), 0); // send connected message
    send(new_socket, fdConn[i].channel->msg, strlen(fdConn[i].channel->msg), 0); // Sending last known message on channel
    
    printf("Connected sent to %d, at channel %s\n", fdConn[i].sd, fdConn[i].channel->name);

    return 0;
}

int handleRequest(int sd, telem_t *channel, conn_t fdConn[]) /// Reads user message and sends it accordingly
{
    struct sockaddr_in client_addr;
    socklen_t size = sizeof(client_addr);
    char buffer[bufSize - 5] = {0};
    
	if (channel == NULL)
	{
		perror("Channel is null :(");
		exit(1);
		return 1;
	}
	
    int valRead = recv(sd, buffer, bufSize, MSG_NOSIGNAL);
    if (valRead == 0) 
    {
        // somebody disconnected, get his details and print
        getpeername(sd, (struct sockaddr *)&client_addr, &size);
        printf("Client %d disconnected. IP: %s:%d\n", sd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        close(sd);
        return 1;
    }
    else // read client message and process it
    {
        buffer[valRead] = '\0';
        if (strcmp(buffer, "0xEND\0") == 0) // codification for connection ended
		{
			printf("Client %d ended connection by writing #.\n", sd);
			
			return 1;
		}
		else
		{
			printf("User %d: %s", sd, buffer);
			char msg[bufSize] = {0};
			sprintf(msg, "|%d|%s", sd, buffer); // process message. |<user_id>|<msg>
			msgPropagation(channel, msg); // set last message
			
			for (int j = 0; j < MAX_USERS; j++)
			{
				// check not to send to the same person who put the message
				// and if the user is connected to a channel who is supposed to receive the message
				if (fdConn[j].sd != 0 && (sd != fdConn[j].sd) && isIndirectChild(fdConn[j].channel, channel))
				{
					send(fdConn[j].sd, msg, strlen(msg), MSG_NOSIGNAL);
				}
			}
		}
        return 0;
    }
    
    return 2; // should not be reached.
}

int initConn(conn_t fdConn[]) // zero out vector
{
    for (int i = 0; i <= MAX_USERS; ++i)
    {
        fdConn[i].sd = 0;
        fdConn[i].channel = NULL;
    }

    return 0;
}

int server()
{
    int fdListen;
    conn_t fdConn[MAX_USERS];
    initConn(fdConn);

    fd_set sockets;
    struct sockaddr_in server_addr = initServer(&fdListen);
    socklen_t size = sizeof(server_addr);

    if (listenServer(fdListen) != 0)
        exit(0);

    int max_sd;


    while (1)
    {
        max_sd = initSSet(&sockets, fdListen, fdConn);

        // select -> iterates through socket descriptors
        // and keeps in sockets only those who are active
        int activity = select(max_sd + 1, &sockets, NULL, NULL, NULL);
        if (activity < 0)
            puts("Error at select");

        // activity on server socket means someone is waiting to be connected
        if (FD_ISSET(fdListen, &sockets))
        {
            if (addNewSocket(fdListen, fdConn, server_addr, size) != 0)
                exit(0);
        }
        else
        // check if someone wants to send a message
        for (int i = 0; i < MAX_USERS; i++)
        {
            int sd = fdConn[i].sd;
            
            // 1-4 system reserved descriptors
            if (sd == 0 || sd < 4 || !FD_ISSET(sd, &sockets))
                continue;


            printf("Activity on socket %d\n", sd);
            if (handleRequest(sd, fdConn[i].channel, fdConn) == 1) // connection closed
            {
                fdConn[i].sd = 0;
                fdConn[i].channel = NULL;
            }
        }
    }
}

void telem_init()
{
    if (!root)
    {
        root = telem_create("root");
    }
}
