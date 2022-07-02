#include "utils.h"

// void echo(int connfd) {
//   char buf[MAXLINE];
//   ssize_t recv_byte;

//   while ((recv_byte = recv(connfd, buf, MAXLINE, 0)) > 0) {
//     fprintf(stderr, "[log] server received %zd\n", recv_byte);
//     send(connfd, buf, recv_byte, 0);
//   }
// }

int main(int ac, char **av) {
  if (ac != 2) {
    fprintf(stderr, "Usage: %s <port>\n", av[0]);
    exit(0);
  }

  int listenfd = open_listenfd(av[1]);
  while (true) {
    struct sockaddr_storage clientaddr;
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    int connfd = accept(listenfd, (t_socketaddr *)&clientaddr, &clientlen);

    char client_hostname[MAXLINE], client_port[MAXLINE];
    getnameinfo((t_socketaddr *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
    printf("connected to (%s, %s)\n", client_hostname, client_port);
    echo(connfd);
    close(connfd);
    break;
  }
  close(listenfd);
  exit(0);
}
