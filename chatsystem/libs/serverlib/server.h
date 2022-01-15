#ifndef foo_h__
#define foo_h__

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/time.h>

/// Struct for simulating the nodes in the arborescent channels
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


void add_child(telem_t *parent, telem_t *child);
void msgPropagation(telem_t *start, const char *msg);
telem_t *search(telem_t *start, telem_t *givenNode);
telem_t *searchParent(telem_t *start, telem_t *givenNode);
int isIndirectChild(telem_t *looking_for, telem_t *current);
void swap(telem_t **swapX, telem_t **swapY);
void telem_destroy(telem_t **node);
void telem_deleteLeaf(telem_t **node);
void telem_delete(telem_t *node);
void telem_insertNode(telem_t *child, telem_t *parent);
void free_list_of_channels(char **channels);
char **get_channels_from_path(char *path, int *num);
telem_t *telem_create_from_path(char *path);
struct sockaddr_in initServer(int *fd);
int listenServer(int fd);
int initSSet(fd_set *sockets, int fd, conn_t fdConn[]);
int addNewSocket(int fdListen, conn_t fdConn[], struct sockaddr_in server_addr, socklen_t size);
int handleRequest(int sd, telem_t *channel, conn_t fdConn[]);
int initConn(conn_t fdConn[]);
int server();
void telem_init();
 
#endif 
