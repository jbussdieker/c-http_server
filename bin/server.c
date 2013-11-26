#include <stdio.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define PORT 5000


///////////////////////////////////////////////////////////////////////////////
// Data structures
///////////////////////////////////////////////////////////////////////////////
struct worker_arg_struct {
  struct sockaddr_in client_addr; 
  int connfd;
};

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////
int start_server(int port);
void start_fork_acceptor(int listen_socket);
void start_thread_acceptor(int listen_socket);
void *server(void *arguments);

///////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////
void print_error(char *msg, int num) {
  printf("%s", msg);
  switch (num) {
    case EADDRINUSE:
      printf(" (Address in use)\n");
      break;
    default:
      printf(" (Unknown %d)\n", num);
      break;
  }
}

void print_addr(struct sockaddr_in *addr) {
  char str[INET_ADDRSTRLEN];
  inet_ntop(addr->sin_family, &(addr->sin_addr), str, INET_ADDRSTRLEN);
  printf("%s:%d\n", str, addr->sin_port);
}

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
  int listenfd;

  listenfd = start_server(PORT);
  start_thread_acceptor(listenfd);

  return 0;
}

int start_server(int port) {
  int result;
  int listenfd;
  struct sockaddr_in src_addr;

  printf("Starting Server...\n");

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&src_addr, 0, sizeof(src_addr));

  src_addr.sin_family = AF_INET;
  src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  src_addr.sin_port = htons(port); 

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

  printf("Listening on port %d...\n", port);

  return listenfd;
}

void start_thread_acceptor(int listenfd) {
  struct worker_arg_struct args;
  int client_addr_len = sizeof(args.client_addr);
  pthread_t child;

  while(1) {
    args.connfd = accept(listenfd, (struct sockaddr *)&args.client_addr, &client_addr_len);
    pthread_create(&child, 0, server, (void *)&args);
    pthread_detach(child);
  }
}

void start_fork_acceptor(int listenfd) {
  struct worker_arg_struct args;
  int client_addr_len = sizeof(args.client_addr);
  int pid;

  while(1) {
    args.connfd = accept(listenfd, (struct sockaddr *)&args.client_addr, &client_addr_len);
    pid = fork();

    // Error
    if (pid < 0) {
      printf("ERROR?\n");
      exit(1);
    }
    // The child
    if (pid == 0) {
      close(listenfd);
      server(&args);
      exit(0);
    }
    // The parent
    if (pid > 0) {
      close(args.connfd);
    }
  }
}

void *server(void *arguments) {
  char buffer[255];
  int read_size, written;
  char *resp;
  struct worker_arg_struct *args = arguments;

  print_addr(&args->client_addr);

  read_size = recv(args->connfd, buffer, 255, 0);

  resp = "HTTP/1.0 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nOK";

  written = send(args->connfd, resp, strlen(resp), 0);

  if (written != strlen(resp)) {
    printf("Error writing\n");
  }

  int result = close(args->connfd);
  if (result != 0) {
    printf("Error closing (%d)\n", result);
  }

  return 0;
}

