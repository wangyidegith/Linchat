#ifndef M_NET_H
#define M_NET_H

#include <sys/types.h>
#include <mqueue.h>

int createListenSocket(const char* ip, unsigned short port);
int createListenUsocket(const char* SOCKET_PATH);
int createConnectSocket(const char* server_ip, int server_port);
int createConnectUsocket(const char* SOCKET_PATH);
int makeSocketNonBlocking(int sockfd);
ssize_t writen(const int fd, const char* buf, const size_t n);
ssize_t readn(const int fd, char* buf, const size_t n);
ssize_t readn_b(const int fd, char* buf, const size_t n);
ssize_t recvn_b(const int fd, char* buf, const size_t n, int flags);
ssize_t recvn(const int fd, char* buf, const size_t n, int flags);

#endif
