#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
  pid_t c_pid;

  char* directory = argv[1]; // Optional parameter

  c_pid = fork();
  if (c_pid == -1) { printf("fork failed.\n"); }

  if (c_pid == 0) { // Child Process
  printf("Currently in child process %d\n", getpid());

  // Listing our the arguments to display all files with extra information
  char* args[] = { "ls", "-l", "-a", directory, NULL};

  execvp("ls", args);
  printf("EXECVP Failed\n");
  } else { // Parent Process
    wait(NULL);
    printf("Currently in parent process (Child process has finished)\n");
  }
}
