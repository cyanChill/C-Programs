#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
  pid_t c_pid;

  c_pid = fork();
  if (c_pid == -1) { printf("fork failed.\n"); }

  if (c_pid == 0) { // Child Process
    printf("Currently in child process %d\n", getpid());
    // The date command is located at /bin/date
    execl("/bin/date", "/bin/date", (char *) NULL);
    printf("EXECL Failed\n");
  } else { // Parent Process
    wait(NULL); // Wait for child process to finish
    printf("Currently in parent process (Child process has finished)\n");
  }
}
