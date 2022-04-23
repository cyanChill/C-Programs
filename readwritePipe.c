#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  int fd, pipefd[2];
  pid_t c_pid;

  // Buffer stuff
  char buf;

  if (argc != 2) { // Check for invalid args
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }  

  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  c_pid = fork();
  if (c_pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (c_pid == 0) {  // Child Process
    // Open the file in read mode
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
      perror("open in read");
      exit(EXIT_FAILURE);
    }

    close(pipefd[0]); // Close read end

    // Repeatingly read & write 1 byte from file
    while (read(fd, &buf, 1) > 0) {
      write(pipefd[1], &buf, 1);
    }

    if (close(fd) < 0) {
      perror("close");
      exit(EXIT_FAILURE);
    }
  } else {  // Parent Process
    wait(NULL);
    // Open the file in append mode
    fd = open(argv[1], O_WRONLY | O_APPEND);
    if (fd == -1) {
      perror("open in append");
      exit(EXIT_FAILURE);
    }

    close(pipefd[1]); // Close write end

    write(fd, "Parent is writing:", 18);
    while (read(pipefd[0], &buf, 1) > 0) {
      write(fd, &buf, 1);
    }

    if (close(fd) < 0) {
      perror("close");
      exit(EXIT_FAILURE);
    }
  }
}
