#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main() {
  pid_t pid, c_pid1, c_pid2;

  // Create first child
  c_pid1 = fork();

  if (c_pid1 == -1) { printf("fork failed.\n"); }

  // Required as we don't want to recreate this in the child process we just created
  if (c_pid1 > 0) { c_pid2 = fork(); }

  if (c_pid2 == -1) { printf("fork failed.\n"); }

  if (c_pid1 == 0) { // Child process
    printf("I am child one, my pid is: %d\n", getpid());
  } else if (c_pid2 == 0) {
    printf("I am child two, my pid is: %d\n", getpid());
  } else { // Parent process
    // Wait for the child processes (in order)
    waitpid(c_pid1, NULL, 0);
    waitpid(c_pid2, NULL, 0);

    printf("I am the parent child processes %d & %d have terminated\n", c_pid1, c_pid2);
  }
}
