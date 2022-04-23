#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int isNumber(char s[]);

int main(int argc, char *argv[]) {
  int pipefd1[2], pipefd2[2];
  pid_t c_pid1, c_pid2;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <integer>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (isNumber(argv[1]) == 0) {
    printf("Please enter in an integer");
    exit(EXIT_FAILURE);
  }

  int num = atoi(argv[1]); // Converts char* to int
  // The buffers that we use to transfer data
  int bufSize = 10;
  char oddSeqSum[bufSize], evenSeqSum[bufSize];

  if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  c_pid1 = fork();
  if (c_pid1 == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (c_pid1 > 0) { c_pid2 = fork(); }
  if (c_pid2 == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (c_pid1 == 0) { 
    // Computes even numbers
    close(pipefd1[0]); // Close read end
    int evensum = 0;
    for (int i = 0; i <= num; i += 2) { evensum += i; }

    printf("Sum of even numbers from 0 to %d computed in child %d: %d\n", num, getpid(), evensum);
    sprintf(evenSeqSum, "%d", evensum); // Moves number into our buffer
    write(pipefd1[1], evenSeqSum, bufSize); // Pass data through the pipe
  } else if (c_pid2 == 0) {
    // Computes odd numbers
    close(pipefd2[0]); // Close read end
    int oddsum = 0;
    for (int i = 1; i <= num; i += 2) { oddsum += i; }

    printf("Sum of odd numbers from 0 to %d computed in child %d: %d\n", num, getpid(), oddsum);
    sprintf(oddSeqSum, "%d", oddsum); // Moves number into our buffer
    write(pipefd2[1], oddSeqSum, bufSize); // Pass data through the pipe
  } else { // Parent process
    // Wait for the child processes (in order)
    waitpid(c_pid1, NULL, 0);
    waitpid(c_pid2, NULL, 0);

    // Close write ends
    close(pipefd1[1]);
    close(pipefd2[1]);

    // Can reuse as we only modified the "instance"
    read(pipefd1[0], evenSeqSum, bufSize);
    read(pipefd2[0], oddSeqSum, bufSize);

    int s1 = atoi(evenSeqSum);
    int s2 = atoi(oddSeqSum);
    int s3 = s1 + s2;
    printf("Sum of all numbers from 0 to %d computed in parent %d: %d\n", num, getpid(), s3);
  }
}

// Function to check whether an input is a number
int isNumber(char s[]) {
  for (int i = 0; s[i] != '\0'; i++) {
    if (isdigit(s[i]) == 0) { return 0; }
  }
  return 1;
}
