#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
  // Parent P
  int a = 10, b = 25, fq = 0, fr = 0;
  
  fq = fork();  // Fork a child - call it Process Q
  if (fq == 0) {  // Child successfully forked
    a = a + b;
    wait(NULL);
    // Prints values of a, b, and process_id
    printf("Currently in process Q (1) with a ppid of %d\n", getppid());
    printf("  > a: %d, b: %d, process_id: %d\n", a, b, getpid());

    fr = fork();  // Fork another child - call it Process R
    if (fr != 0) {
      b = b + 20;
      printf("Currently in process Q (2) with a ppid of %d\n", getppid());
      printf("  > a: %d, b: %d, process_id: %d\n", a, b, getpid());
    } else {
      a = a * b + 30;
      printf("Currently in process R with a ppid of %d\n", getppid());
      printf("  > a: %d, b: %d, process_id: %d\n", a, b, getpid());
    }
  } else {
    b = a + b - 5;
    printf("Currently in process P with a ppid of %d\n", getppid());
    printf("  > a: %d, b: %d, process_id: %d\n", a, b, getpid());

    // Make sure the child process finishes before terminating
    wait(NULL);
  }
}
