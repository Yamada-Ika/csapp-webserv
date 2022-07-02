#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/stat.h>

typedef struct addrinfo t_socketaddrinfo;
typedef struct sockaddr t_socketaddr;

// typedef struct s_socket {
//   t_socketaddrinfo *info;
//   t_socketaddr     *addr;
//   int              descriptor;
// } t_socket;

#define MAXLINE 1024

#ifndef DEBUG
#define DEBUG true
#endif

void echo(int connfd);
int wrp_socket(t_socketaddrinfo *addr);
int wrp_bind(int socket_fd, t_socketaddrinfo* addr);
int open_clientfd(const char *hostname, const char *port);
int open_listenfd(const char *port);

#endif
