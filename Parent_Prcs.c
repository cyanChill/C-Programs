#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
  pid_t c_pid1, c_pid2;

  if (argc != 3) { // Check for invalid args
    fprintf(stderr, "Usage: %s <exe 1> <exe 2>\nNote: Executable names should start with './'\n", argv[0]);
    exit(EXIT_FAILURE);
  }  

  char* exe1 = argv[1];
  char* exe2 = argv[2];

  c_pid1 = fork();
  if (c_pid1 == -1) { printf("fork failed.\n"); }

  if (c_pid1 == 0) { // Child 1 Code
    printf("In Child 1's (pid = %d) code\n", getpid());
    char* args[] = {exe1, (char *) NULL};
    execvp(exe1, args);
    printf("Child 1 EXECV Failed\n");
  }

  if (c_pid1 > 0) {
    waitpid(c_pid1, NULL, 0); // Wait for Child 1 code to finish
    c_pid2 = fork();
    if (c_pid2 == -1) { printf("fork failed.\n"); }
  }

  if (c_pid1 > 0 && c_pid2 == 0) { // Child 2 Code
    printf("In Child 2's (pid = %d) code\n", getpid());
    char* args[] = {exe2, (char *) NULL};
    execvp(exe2, args);
    printf("Child 2 EXECV Failed\n");

    return 0; // Finish Child Process
  }

  if (c_pid1 > 0 & c_pid2 > 0) { // Parent's Code
    waitpid(c_pid2, NULL, 0); // Wait for Child 2 code to finish
    printf("All child processes have finished\n");
  }
}
