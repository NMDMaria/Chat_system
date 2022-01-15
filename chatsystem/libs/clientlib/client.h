#ifndef foo_h__
#define foo_h__

int initClient(int *fd);
void printBuffer(char *buffer);
void printLastMessage(int *fd);
int testConnection(int *fd);
void readRequest(int *fd);
int writeRequest(int *fd);
void sublisher();
void publisher();
void subscriber();

#endif 
