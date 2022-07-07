#include <sys/epoll.h>
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
#define MAX_EVENTS 10
#define MAXLINE 4096

void panic(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void echo(int connfd) {
  char buf[MAXLINE];
  ssize_t recv_byte;

  // recv_byte = recv(connfd, buf, MAXLINE, 0);
  // send(connfd, buf, recv_byte, 0);

  while ((recv_byte = recv(connfd, buf, MAXLINE, 0)) > 0) {
    if (strcmp(buf, "quit\r\n") == 0) {
      break;
    }
    send(connfd, buf, recv_byte, 0);
  }
}

void do_use_fd(int connfd) {
  echo(connfd);
}

int open_listenfd(const char *port) {
  struct addrinfo hints, *listp;
  bzero(&hints, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;              // Connections only
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;  // Accept connections
  hints.ai_flags |= AI_NUMERICSERV;             // Using port
  int rc;
  if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
    panic(gai_strerror(rc));
  }

  int listenfd = -1;
  for (struct addrinfo* p = listp; p; p = p->ai_next) {
    if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      continue;
    }
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
      break;
    }
    close(listenfd);
  }
  freeaddrinfo(listp);

  if (listen(listenfd, 1024) < 0) {
    close(listenfd);
    return -1;
  }

  return listenfd;
}

void setnonblocking(int fd) {
  if (fcntl(fd, F_SETFL, O_NONBLOCK) != 0) {
    panic("Error: Failed to set non-blocking socket");
  }
}

int main(void) {
  int listen_sock, conn_sock, nfds, epollfd;
  struct epoll_event ev, events[MAX_EVENTS];

  listen_sock = open_listenfd("80");

  epollfd = epoll_create(0);
  if (epollfd == -1) {
    panic("hoge");
  }

  ev.events = EPOLLIN;
  ev.data.fd = listen_sock;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
    panic("hoge");
  }

  while (true) {
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      panic("hoge");
    }

    #if DEBUG
    fprintf(stderr, "[log] connections : %d\n", nfds);
    #endif

    for (int n = 0; n < nfds; ++n) {
      if (events[n].data.fd == listen_sock) {
        struct sockaddr_storage clientaddr;
        socklen_t clientlen = sizeof(struct sockaddr_storage);
        conn_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &clientlen);
        if (conn_sock == -1) {
          panic("Error: Falide to accept");
        }
        setnonblocking(conn_sock);
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = conn_sock;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
          panic("Error: Failed to add fd to epoll instance");
        }
      } else {
        do_use_fd(events[n].data.fd);
      }
    }
  }
}
