#include "utils.h"

#define logd(d) fprintf(stderr, "[log] %d\n", d)

int main(void) {
  {
    int clientfd;
    clientfd = open_clientfd("localhost", "8080");
    logd(clientfd);
    close(clientfd);
  }

  {
    int listenfd;
    listenfd = open_listenfd("8080");
    logd(listenfd);
    close(listenfd);
  }
}
