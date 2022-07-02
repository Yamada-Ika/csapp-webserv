#include "utils.h"

// int wrap_socket(t_socket *sock) {
//   return socket(sock->info->ai_family, sock->info->ai_socktype, sock->info->ai_protocol);
// }

// int wrap_bind(t_socket *sock) {
//   return bind(sock->descriptor, sock->info->ai_addr, sock->info->ai_addr);
// }

// int wrap_connect(t_socket *sock) {
//   return connect(sock->descriptor, sock->info->ai_addr, sock->info->ai_addrlen);
// }

void echo(int connfd) {
  char buf[MAXLINE];
  ssize_t recv_byte;

  while ((recv_byte = recv(connfd, buf, MAXLINE, 0)) > 0) {
    fprintf(stderr, "[log] server received %zd\n", recv_byte);
    if (strncmp(buf, "quit", 4) == 0) {
      return;
    }
    send(connfd, buf, recv_byte, 0);
  }
}

int wrp_socket(t_socketaddrinfo *addr) {
  return socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
}

int wrp_bind(int socket_fd, t_socketaddrinfo* addr) {
  return bind(socket_fd, addr->ai_addr, addr->ai_addrlen);
}

int wrp_connect(int socket_fd, t_socketaddrinfo* addr) {
  return connect(socket_fd, addr->ai_addr, addr->ai_addrlen);
}

int open_clientfd(const char *hostname, const char *port) {
  // Get socket addres list
  t_socketaddrinfo hints, *listp;
  bzero(&hints, sizeof(t_socketaddrinfo));
  hints.ai_socktype = SOCK_STREAM;  // Connections only
  hints.ai_flags = AI_NUMERICSERV;  // Using a numeric port
  hints.ai_flags |= AI_ADDRCONFIG;  // Recommended
  int rc;
  if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
    fprintf(stderr, "Error: %s\n", gai_strerror(rc));
    exit(1);
  }

  // Try to connect
  int clientfd = -1;
  for (t_socketaddrinfo* p = listp; p; p = p->ai_next) {
    if ((clientfd = wrp_socket(p)) < 0) {
      continue;
    }
    if (wrp_connect(clientfd, p) != -1) {
      break;
    }
    close(clientfd);
  }
  freeaddrinfo(listp);
  return clientfd;
}

int open_listenfd(const char *port) {
  // Get socket addres list
  t_socketaddrinfo hints, *listp;
  bzero(&hints, sizeof(t_socketaddrinfo));
  hints.ai_socktype = SOCK_STREAM;              // Connections only
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;  // Accept connections
  hints.ai_flags |= AI_NUMERICSERV;             // Using port
  int rc;
  if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
    fprintf(stderr, "Error: %s\n", gai_strerror(rc));
    exit(1);
  }

  int listenfd = -1;
  for (t_socketaddrinfo* p = listp; p; p = p->ai_next) {
    if ((listenfd = wrp_socket(p)) < 0) {
      continue;
    }
    // Avoid already address bind
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

    if (wrp_bind(listenfd, p) == 0) {
      break;
    }
    close(listenfd);
  }
  freeaddrinfo(listp);

  if (listen(listenfd, 0) < 0) {
    close(listenfd);
    return -1;
  }

  return listenfd;
}
