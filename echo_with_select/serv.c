#include "utils.h"

void command(void) {
  char buf[MAXLINE];
  if (!fgets(buf, MAXLINE, stdin)) {
    // exit(0);
    return;
  }
  printf("%s", buf);
}

int main(int ac, char **av) {
  if (ac != 2) {
    fprintf(stderr, "Usage: %s <port>\n", av[0]);
    exit(0);
  }

  int listenfd = open_listenfd(av[1]);
  assert(listenfd >= 0);

  fd_set read_set, ready_set;
  FD_ZERO(&read_set);
  FD_SET(STDIN_FILENO, &read_set);
  FD_SET(listenfd, &read_set);

  while (true) {
    ready_set = read_set;
    select(listenfd+1, &ready_set, NULL, NULL, NULL);
    if (FD_ISSET(STDIN_FILENO, &ready_set)) {
      command();
    }
    if (FD_ISSET(listenfd, &ready_set)) {
      struct sockaddr_storage clientaddr;
      socklen_t clientlen = sizeof(struct sockaddr_storage);
      int connfd = accept(listenfd, (t_socketaddr *)&clientaddr, &clientlen);
      assert(connfd >= 0);
      echo(connfd);
      close(connfd);
    }
    printf("!!!!\n");
  }
}

