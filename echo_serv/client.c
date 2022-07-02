#include "utils.h"

char *read_line(char *buf, int len, FILE *stream) {
  printf("client << ");
  return fgets(buf, len, stream);
}

int main(int ac, char **av) {
  if (ac != 3) {
    fprintf(stderr, "Usage: %s <host> <port>\n", av[0]);
    exit(0);
  }

  char *host = av[1], *port = av[2];
  int clientfd = open_clientfd(host, port);
  if (clientfd == -1) {
    fprintf(stderr, "Error: Failed to open socket\n");
    exit(1);
  }

  char buf[MAXLINE];
  char recbuf[MAXLINE];
  while (read_line(buf, MAXLINE, stdin) != NULL) {
    send(clientfd, buf, strlen(buf), 0);
    ssize_t byte = recv(clientfd, recbuf, MAXLINE, 0);
    recbuf[byte] = '\0';
    printf("server >> %s", recbuf);
  }
  close(clientfd);
  exit(0);
}
