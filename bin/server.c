#include <stdio.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define PORT 5000

int listenfd;

void print_error(char *msg, int num) {
  printf(msg);
  switch (num) {
    case EADDRINUSE:
      printf(" (Address in use)\n");
      break;
    default:
      printf(" (Unknown %d)\n", num);
      break;
  }
}

void server(int connfd) {
  char buffer[255];
  //printf(" * [%d] Connection\n", connfd);
  int read_size = read(connfd, buffer, 255);
  //printf("%.*s\n", read_size, buffer);

  char *resp = "HTTP/1.0 200 OK\r\nContent-Length: 0\r\n\r\n";
  int written = send(connfd, resp, strlen(resp), 0);

  //printf("%.*s\n", written, resp);

  //printf(" * [%d] Connection Write %d\n", connfd, written);
  int result = close(connfd);
  if (result != 0) {
    printf("Error closing (%d)\n", result);
  }

  //printf(" * [%d] Connection Closed\n", connfd);
}

void sig_handler(int signo)
{
  if (signo == SIGINT) {
    printf("Shutting down..\n");
    close(listenfd);
    exit(0);
  }
}

int main(int argc, char **argv) {
  int result;
  struct sockaddr_in src_addr;

  printf("Starting Server...\n");

  if (signal(SIGINT, sig_handler) == SIG_ERR)
    printf("\ncan't catch SIGINT\n");

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&src_addr, 0, sizeof(src_addr));

  src_addr.sin_family = AF_INET;
  src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  src_addr.sin_port = htons(PORT); 

  result = bind(listenfd, (struct sockaddr*)&src_addr, sizeof(src_addr));
  if (result != 0) {
    print_error("Error binding", errno);
    exit(1);
  }

  result = listen(listenfd, 5);
  if (result != 0) {
    print_error("Error listening", errno);
    exit(1);
  }

  printf("Listening on port %d...\n", PORT);

  while(1) {
    int connfd;
    connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
    int pid = fork();
    if (pid < 0) {
      printf("ERROR?\n");
      break;
    }
    // The child
    if (pid == 0) {
      server(connfd);
      exit(1);
    }
    // The parent
    if (pid > 0) {
      //int x;
      //waitpid(-1,&x,WNOHANG);
      close(connfd);
    }
  }

  return 0;
}
