#include "utils.h"

typedef struct s_pool {
  int maxfd;
  fd_set read_set;
  fd_set ready_set;
  int nready;
  int maxi; // ?
  int clientfd[FD_SETSIZE];
} t_pool;

void init_pool(int listenfd, t_pool *p) {
  p->maxi = -1;
  for (int i = 0; i < FD_SETSIZE; ++i) {
    p->clientfd[i] = -1;
  }
  p->maxfd = listenfd;
  FD_ZERO(&p->read_set);
  FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, t_pool *p) {
  p->nready--;
  int i;
  for (i = 0; i < FD_SETSIZE; i++) {
    if (p->clientfd[i] < 0) { // fill at empty slot
      p->clientfd[i] = connfd;
      FD_SET(connfd, &p->read_set);
      if (connfd > p->maxfd) {
        p->maxfd = connfd;
      }
      if (i > p->maxi) {
        p->maxi = i;
      }
      break;
    }
  }
  if (i == FD_SETSIZE) {
    // app_error("app_client error: Too many clients");
    fprintf(stderr, "app_client error: Too many clients");
  }
}

void check_clients(t_pool *p) {
  char buf[MAXLINE];
  int connfd;
  ssize_t recvbyte;

  for (int i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
    connfd = p->clientfd[i];
    assert(connfd >= 0);
    if (FD_ISSET(connfd, &p->ready_set)) { // fd_setが更新されるのはselectがコールされた時だけでは?
      p->nready--;
      // while でループさせずに一回だけecho
      if ((recvbyte = recv(connfd, buf, MAXLINE, 0)) > 0) {
        printf("server >> received size %zd\n", recvbyte);
        printf("server >> request\n");
        buf[recvbyte]='\0';
        printf("%s", buf);
        // printf("%*.s", recvbyte, buf);

        sprintf(buf, "HTTP/1.1 200 OK\r\n");
        sprintf(buf, "Content-Length: 88\r\n");
        sprintf(buf, "Content-Type: text/html\r\n");
        sprintf(buf, "Connection: Closed\r\n\r\n");
        sprintf(buf, "<html><body><h1>Hello, World!</h1></body></html>\r\n");
        send(connfd, buf, strlen(buf), 0);
        // close(connfd);
        // FD_CLR(connfd, &p->read_set);
        // p->clientfd[i] = -1;
      } else {
        close(connfd);
        FD_CLR(connfd, &p->read_set);
        p->clientfd[i] = -1;
      }
      // echo(connfd);
      // close(connfd);
      // FD_CLR(connfd, &p->read_set);
      // p->clientfd[i] = -1;
    }
  }
}

int main(int ac, char **av) {
  if (ac != 2) {
    fprintf(stderr, "Usage: %s <port>\n", av[1]);
    exit(0);
  }

  static t_pool pool;
  int listenfd = open_listenfd(av[1]);
  init_pool(listenfd, &pool);

  while (true) {
    pool.ready_set = pool.read_set;
    pool.nready = select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

    if (FD_ISSET(listenfd, &pool.ready_set)) {
      struct sockaddr_storage clientaddr;
      socklen_t clientlen = sizeof(struct sockaddr_storage);
      int connfd = accept(listenfd, (t_socketaddr *)&clientaddr, &clientlen);
      add_client(connfd, &pool);
    }
    check_clients(&pool);
  }
}
