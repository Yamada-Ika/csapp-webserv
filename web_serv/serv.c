#include "utils.h"

void clienterror(int fd, const char *caurse, const char *errnum,
    const char *shortmsg, const char *longmsg) {
  char body[MAXLINE], buf[MAXLINE];

  // Make response body
  sprintf(body, "<html><title>surume Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, caurse);
  sprintf(body, "%s<hr><em>The surume web server</em>\r\n", body);

  // Print HTTP response
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  send(fd, buf, strlen(buf), 0);
  sprintf(buf, "Content-type: text/html\r\n");
  send(fd, buf, strlen(buf), 0);
  sprintf(buf, "Content-length: %zu\r\n", strlen(body));
  send(fd, buf, strlen(buf), 0);
  send(fd, body, strlen(body), 0);
}

// TODO bool返すの微妙かな
bool parse_uri(char *uri, char *filename, char *query) {
  char *sep_at;

  if (strstr(uri, "cgi-bin") == NULL) { // static content
    strcpy(query, "");
    strcpy(filename, "."); // current directory
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/') {
      strcat(filename, "index.html");
    }
    return true;
  } else { // dynamic content
    sep_at = strstr(uri, "?");

    #if DEBUG
    fprintf(stderr, "[log] sep_at : %s\n", sep_at);
    #endif

    if (sep_at != NULL) {
      strcpy(query, sep_at + 1);
      snprintf(query, strlen(sep_at + 1) + 1, "%s", sep_at + 1);
      #if DEBUG
      fprintf(stderr, "[log] query  : %s\n", query);
      #endif
      *sep_at = '\0'; // いる？
    } else {
      strcpy(query, "");
    }
    strcpy(filename, ".");
    strcat(filename, uri);
    return false;
  }
  return false;
}

void get_filetype(const char *name, char *type) {
  if (strstr(name, ".html")) {
    strcpy(type, "text/html");
  } else if (strstr(name, ".gif")) {
    strcpy(type, "image/gif");
  } else if (strstr(name, ".jpg")) {
    strcpy(type, "image/jpg");
  } else {
    strcpy(type, "text/plain");
  }
}

void serve_static(const int fd, const char *filename, off_t filesize) {
  // send response headers to client
  char filetype[MAXLINE], buf[MAXLINE];
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: surume Web Server\r\n", buf);
  sprintf(buf, "%sContent-length: %lld\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  send(fd, buf, strlen(buf), 0);

  // send response body to client
  int srcfd = open(filename, O_RDONLY, 0);
  char *srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  close(srcfd);
  send(fd, srcp, filesize, 0);
  munmap(srcp, filesize);
}

extern char **environ;

void serve_dynamic(const int fd, const char *filename, const char* query) {
  char buf[MAXLINE];

  // return first part of HTTP response
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  send(fd, buf, strlen(buf), 0);
  sprintf(buf, "Server: surume Web Server\r\n");
  send(fd, buf, strlen(buf), 0);

  if (fork() == 0) { // child
    // real server would set all CGI vars here
    setenv("QUERY_STRING", query, 1);
    dup2(fd, STDOUT_FILENO);
    execve(filename, NULL, environ);
  }
  wait(NULL);
}

void start_transact(const int connfd) {
  char reqbuf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

  // Read http request
  ssize_t reqbyte;
  reqbyte = recv(connfd, reqbuf, MAXLINE, 0);

#if DEBUG
  fprintf(stderr, "[log] Received byte : %zd\n", reqbyte);
#endif

  fprintf(stderr, "[log] >>> HTTP request\n");
  fprintf(stderr, "%s\n", reqbuf);
  fprintf(stderr, "[log] <<< HTTP request\n");

  sscanf(reqbuf, "%s %s %s", method, uri, version);

  #if DEBUG
  fprintf(stderr, "[log] method  : %s\n", method);
  fprintf(stderr, "[log] uri     : %s\n", uri);
  fprintf(stderr, "[log] version : %s\n", version);
  #endif

  if (strcasecmp(method, "GET") != 0) {
    clienterror(connfd, method, "501", "Not Implemented", "surume does not implement this method");
    return;
  }

  char filename[MAXLINE], query[MAXLINE];
  bool should_respond_static = parse_uri(uri, filename, query);

  #if DEBUG
  fprintf(stderr, "[log] uri      : %s\n", uri);
  fprintf(stderr, "[log] filename : %s\n", filename);
  fprintf(stderr, "[log] query    : %s\n", query);
  fprintf(stderr, "[log] 0:dynamic 1:static : %d\n", should_respond_static);
  #endif

  struct stat sbuf;
  if (stat(filename, &sbuf) < 0) {
    #if DEBUG
    fprintf(stderr, "[log] %s cannot found\n", filename);
    #endif
    clienterror(connfd, filename, "404", "Not found", "surume couldn't find this file");
    return;
  }

  if (should_respond_static) {
    if (!(S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode))) {
      clienterror(connfd, filename, "403", "Forbidden", "surume couldn't read the file");
      return;
    }
    serve_static(connfd, filename, sbuf.st_size);
    #if DEBUG
    fprintf(stderr, "[log] filename : %s\n", filename);
    #endif
  } else {
    if (!(S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode))) {
      #if DEBUG
      fprintf(stderr, "[log] can not run cgi program\n");
      #endif
      clienterror(connfd, filename, "403", "Forbidden", "surume couldn't run the CGI program");
    }
    #if DEBUG
    fprintf(stderr, "[log] filename : %s\n", filename);
    fprintf(stderr, "[log] query    : %s\n", query);
    #endif
    serve_dynamic(connfd, filename, query);
  }
}

void assert_with_errno(bool cond) {
  if (!cond) {
    perror("error");
    assert(cond);
  }
}

int main(int ac, char **av) {
  if (ac != 2) {
    fprintf(stderr, "Usage: %s <port>\n", av[0]);
    exit(0);
  }

  char *port = av[1];
  int listenfd = open_listenfd(port);
  fprintf(stderr, "[log] Listen to socket : %d\n", listenfd);
  assert_with_errno(listenfd >= 0);

  while (true) {
    struct sockaddr_storage clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    int connfd = accept(listenfd, (t_socketaddr *)&clientaddr, &clientlen);

#if DEBUG
    fprintf(stderr, "[log] Listen to socket : %d\n", connfd);
#endif
    assert_with_errno(connfd >= 0);

    char hostname[MAXLINE];
    getnameinfo((t_socketaddr *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    fprintf(stderr, "[log] Accepted connection from (%s, %s)\n", hostname, port);
    start_transact(connfd);
    close(connfd);
  }

  exit(0);
}
